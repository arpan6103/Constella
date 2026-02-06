#include "server.h"
#include "pch.h"

Server::Server(int port, Storage& storage)
    : port_(port), storage_(storage){}

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

        if(command=="PUT"){
            std::string key,value;
            ss>>key>>value;
            if(key.empty() || value.empty()){
                response="ERROR\n";
            }
            else{
                storage_.put(key,value);
                response="OK\n";
            }
        }
        else if(command=="GET"){
            std::string key,value;
            ss>>key;
            if(storage_.get(key,value)){
                response="VALUE: "+value+"\n";
            }
            else{
                response="NOT FOUND\n";
            }
        }
        else{
            response="UNKNOWN COMMAND\n";
        }

        write(client_fd,response.c_str(),response.size());
    }
    close(client_fd);
}