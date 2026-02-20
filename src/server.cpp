#include "server.h"
#include "pch.h"

Server::Server(int port, const std::string& node_id,const std::vector<std::string>& cluster_nodes, Storage& storage,int replication_factor)
    :   port_(port),
        node_id_(node_id), 
        storage_(storage),
        replication_factor_(replication_factor){
        for(const auto& node:cluster_nodes){
            ring_.add_node(node);
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
    std::cout<<"server_fd: ";
    std::cout<<server_fd_<<"\n";
    std::cout<<"Constella node listening on port "<<port_<<"\n";

    while(true){
        int client_fd=accept(server_fd_,nullptr,nullptr);
        std::cerr<<"client_fd: ";
        std::cerr<<client_fd;
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

        std::cout<<"inside handle_client, request: "<<request<<" command: "<<command<<" end\n";

        if(command=="PUT"){
            std::string key,value,request_id;
            ss>>key>>value>>request_id;
            if(key.empty() || value.empty()){
                response="ERROR\n";
            }
            else{
                bool is_replica=!request_id.empty();
                if(!is_replica){
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
                storage_.put(key,value);
                if(!is_replica){
                    auto replicas=ring_.get_replicas(key,replication_factor_);
                    for(const auto& node:replicas){
                        if(node==node_id_){
                            continue;
                        }
                        std::string replica_request="PUT "+key+" "+value+" "+request_id+"\n";
                        forward_request(node,replica_request);
                    }
                }
                response="OK\n";
            }
        }
        else if(command=="GET"){
            std::string key,value;
            ss>>key;
            if(key.empty()){
                response="ERROR\n";
            }
            else{
                auto replicas=ring_.get_replicas(key,replication_factor_);
                bool found=false;
                std::cout<<"replicas for key ";
                for(auto& r:replicas){
                    std::cout<<r<<" ";
                }
                std::cout<<"\n";

                for(const auto& node:replicas){
                    if(node==node_id_){
                        if(storage_.get(key,value)){
                            response="VALUE: "+value+"\n";
                            found=true;
                            break;
                        }
                    }
                    else{
                        std::string resp=forward_request(node,request);
                        if(resp.rfind("VALUE:",0)==0){
                            response=resp;
                            found=true;
                            break;
                        }
                    }
                }
                if(!found){
                    response="NOT FOUND\n";
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

std::string Server::forward_request(const std::string& owner,const std::string& request){
    std::cout<<"inside forward request\n";
    size_t colon=owner.find(':');
    std::string ip=owner.substr(0,colon);
    int port=std::stoi(owner.substr(colon+1));

    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0){
        return "ERROR\n";
    }
    sockaddr_in addr{};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_pton(AF_INET,ip.c_str(),&addr.sin_addr);

    if(connect(sock,(struct sockaddr*)&addr,sizeof(addr))<0){
        close(sock);
        return "ERROR\n";
    }
    write(sock,request.c_str(),request.size());

    char buffer[1024];
    ssize_t bytes=read(sock,buffer,sizeof(buffer)-1);

    close(sock);

    if(bytes<=0){
        return "ERROR\n";
    }
    buffer[bytes]='\0';
    return std::string(buffer);
}

std::string Server::generate_request_id(){
    return node_id_+"_"+std::to_string(request_counter_++);
}