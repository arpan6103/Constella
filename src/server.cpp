#include "server.h"
#include "pch.h"

Server::Server(int port, const std::string& node_id,const std::vector<std::string>& cluster_nodes, Storage& storage,int replication_factor,int write_quorum,int read_quorum)
    :   port_(port),
        node_id_(node_id), 
        storage_(storage),
        replication_factor_(replication_factor),
        write_quorum_(write_quorum),
        read_quorum_(read_quorum),
        cluster_nodes_(cluster_nodes)
        {
            for(const auto& node:cluster_nodes){
                ring_.add_node(node);
                node_alive_[node]=true;
            }
        }

void Server::start(){
    server_fd_=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd_<0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int opt=1;

    setsockopt(server_fd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(port_);

    if(bind(server_fd_,(struct sockaddr*)& address,sizeof(address))<0){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if(listen(server_fd_,10)<0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout<<"Constella node listening on port "<<port_<<"";
    std::thread(&Server::heartbeat_loop,this).detach();

    while(true){
        int client_fd=accept(server_fd_,nullptr,nullptr);
        if(client_fd<0){
            perror("accept");
            continue;
        }
        std::thread(&Server::handle_client,this,client_fd).detach();
    }
}

bool Server::send_message(int fd, const std::string& msg){
    uint32_t len=htonl(static_cast<uint32_t>(msg.size()));
    if(write(fd,&len,4)!=4){
        return false;
    }
    ssize_t sent=write(fd,msg.c_str(),msg.size());
    return sent==static_cast<ssize_t>(msg.size());
}
std::string Server::read_message(int fd){
    uint32_t len_net=0;
    ssize_t n=read(fd,&len_net,4);
    if(n!=4){
        return "";
    }
    uint32_t len=ntohl(len_net);
    if(len==0 || len>1*1024*1024){
        return "";
    }
    std::string msg(len,'\0');
    size_t received=0;
    while(received<len){
        ssize_t r=read(fd,&msg[received],len-received);
        if(r<=0){
            return "";
        }
        received+=r;
    }
    return msg;
}

void Server::handle_client(int client_fd){
    while(true){
        std::string request=read_message(client_fd);
        if(request.empty()){
            break;
        }
        std::stringstream ss(request);
        std::string command;
        ss>>command;
        std::string response;

        if(command=="PING"){
            response="PONG";
        }

        else if(command=="PUT"){
            std::string key,value,request_id;
            ss>>key>>value>>request_id;
            if(key.empty() || value.empty()){
                response="ERROR";
            }
            else{
                bool is_coordinator=request_id.empty();
                if(is_coordinator){
                    request_id=generate_request_id();
                }
                {
                    std::lock_guard<std::mutex>lock(processed_mutex_);
                    if(processed_requests_.count(request_id)){
                        send_message(client_fd, "OK");
                        continue;
                    }
                    processed_requests_.insert(request_id);
                }
                std::vector<std::string>replicas;
                {
                    std::lock_guard<std::mutex>lock(ring_mutex_);
                    replicas=ring_.get_replicas(key,replication_factor_);
                }

                std::cout<<" replicas for: "<< node_id_<<" ";
                for(auto node:replicas){
                    std::cout<<node<< " ";
                }
                std::cout<<"";

                if(is_coordinator){
                    int success_count=0;
                    for(const auto& node:replicas){
                        if(node==node_id_){
                            storage_.put(key,value);
                            success_count++;
                        }
                        else{
                            std::string replica_request="PUT "+key+" "+value+" "+request_id+"";
                            std::string resp=forward_request(node,replica_request);
                            if(resp=="OK"){
                                success_count++;
                            }
                        }
                    }
                    if(success_count>=write_quorum_){
                        response="OK";
                    }
                    else{
                        response="ERROR";
                    }
                }
                else{
                    storage_.put(key,value);
                    response="OK";
                }
            }
        }

        else if(command=="GET"){
            std::string key,request_id;
            ss>>key>>request_id;
            if(key.empty()){
                response="ERROR";
            }
            else{
                bool is_coordinator=request_id.empty();
                if(is_coordinator){
                    request_id=generate_request_id();
                }
                std::vector<std::string>replicas;
                {
                    std::lock_guard<std::mutex>lock(ring_mutex_);
                    replicas=ring_.get_replicas(key,replication_factor_);
                }
                int success_count=0;
                std::string final_value;
                for(const auto& node:replicas){
                    std::string resp;
                    if(node==node_id_){
                        std::string value;
                        if(storage_.get(key,value)){
                            resp="VALUE: "+value+"";
                        }
                        else{
                            resp="NOT FOUND";
                        }
                    }
                    else{
                        if(is_coordinator){
                            std::string forwarded="GET "+key+" "+request_id+"";
                            resp=forward_request(node,forwarded);
                        }
                        else{
                            continue;
                        }
                    }
                    if(resp.rfind("VALUE:",0)==0){
                        success_count++;
                        final_value=resp;
                    }
                    if(success_count>=read_quorum_){
                        break;
                    }
                }
                if(success_count>=read_quorum_){
                    response=final_value;
                }
                else{
                    response="ERROR";
                }
            }
        }
        else{
            response="UNKNOWN COMMAND";
        }
        send_message(client_fd,response);
    }
    close(client_fd);
}

std::string Server::forward_request(const std::string& owner, const std::string& request) {
    size_t colon = owner.find(':');
    std::string host = owner.substr(0, colon);
    std::string port_str = owner.substr(colon + 1);

    struct addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0) {
        return "ERROR";
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) { freeaddrinfo(res); return "ERROR"; }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        freeaddrinfo(res); close(sock); return "ERROR";
    }
    freeaddrinfo(res);
    
    if(!send_message(sock,request)){
        close(sock);
        return "ERROR";
    }
    std::string response=read_message(sock);
    close(sock);
    return response.empty() ? "ERROR" : response;
}

std::string Server::generate_request_id(){
    return node_id_+"_"+std::to_string(request_counter_++);
}

void Server::heartbeat_loop(){
    while(true){
        for(auto& node:cluster_nodes_){
            if(node==node_id_){
                continue;
            }
            std::string response=forward_request(node,"PING");
            update_node_status(node,response=="PONG");
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void Server::update_node_status(const std::string& node, bool alive){
    std::lock_guard<std::mutex>lock(ring_mutex_);
    bool was_alive=node_alive_[node];
    if(was_alive==alive){
        return;
    }
    node_alive_[node]=alive;
    if(alive){
        ring_.add_node(node);
        std::cout<<"Node recovered: "<< node<<"";
    }
    else{
        ring_.remove_node(node);
        std::cout<<"Node unavailable: "<<node<<"";
    }
}