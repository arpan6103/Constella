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
    std::cout<<"Constella node listening on port "<<port_<<"\n";
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

void Server::handle_client(int client_fd){
    char buffer[1024];

    while(true){
        ssize_t bytes=read(client_fd,buffer,sizeof(buffer)-1);
        if(bytes<=0){
            break;
        }
        buffer[bytes]='\0';
        std::string request(buffer);
        std::stringstream ss(request);
        std::string command;
        ss>>command;
        std::string response;

        if(command=="PING"){
            response="PONG\n";
        }

        else if(command=="PUT"){
            std::string key,value,request_id;
            ss>>key>>value>>request_id;
            if(key.empty() || value.empty()){
                response="ERROR\n";
            }
            else{
                bool is_coordinator=request_id.empty();
                if(is_coordinator){
                    request_id=generate_request_id();
                }
                {
                    std::lock_guard<std::mutex>lock(processed_mutex_);
                    if(processed_requests_.count(request_id)){
                        response="OK\n";
                        write(client_fd,response.c_str(),response.size());
                        continue;
                    }
                    processed_requests_.insert(request_id);
                }
                std::vector<std::string>replicas;
                {
                    std::lock_guard<std::mutex>lock(ring_mutex_);
                    replicas=ring_.get_replicas(key,replication_factor_);
                }

                std::cout<<"\n replicas for: "<< node_id_<<" \n";
                for(auto node:replicas){
                    std::cout<<node<< " ";
                }
                std::cout<<"\n";

                if(is_coordinator){
                    int success_count=0;
                    for(const auto& node:replicas){
                        if(node==node_id_){
                            storage_.put(key,value);
                            success_count++;
                        }
                        else{
                            std::string replica_request="PUT "+key+" "+value+" "+request_id+"\n";
                            std::string resp=forward_request(node,replica_request);
                            if(resp=="OK\n"){
                                success_count++;
                            }
                        }
                    }
                    if(success_count>=write_quorum_){
                        response="OK\n";
                    }
                    else{
                        response="ERROR\n";
                    }
                }
                else{
                    storage_.put(key,value);
                    response="OK\n";
                }
            }
        }

        else if(command=="GET"){
            std::string key,request_id;
            ss>>key>>request_id;
            if(key.empty()){
                response="ERROR\n";
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
                            resp="VALUE: "+value+"\n";
                        }
                        else{
                            resp="NOT FOUND\n";
                        }
                    }
                    else{
                        if(is_coordinator){
                            std::string forwarded="GET "+key+" "+request_id+"\n";
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
                    response="ERROR\n";
                }
            }
        }
        else{
            response="UNKNOWN COMMAND\n";
        }

        write(client_fd,response.c_str(),response.size());
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
        return "ERROR\n";
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) { freeaddrinfo(res); return "ERROR\n"; }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        freeaddrinfo(res); close(sock); return "ERROR\n";
    }
    freeaddrinfo(res);

    write(sock, request.c_str(), request.size());
    char buffer[1024];
    ssize_t bytes = read(sock, buffer, sizeof(buffer) - 1);
    close(sock);
    if (bytes <= 0) return "ERROR\n";
    buffer[bytes] = '\0';
    return std::string(buffer);
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
            std::string response=forward_request(node,"PING\n");
            update_node_status(node,response=="PONG\n");
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
        std::cout<<"Node recovered: "<< node<<"\n";
    }
    else{
        ring_.remove_node(node);
        std::cout<<"Node unavailable: "<<node<<"\n";
    }
}