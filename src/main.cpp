#include "storage.h"
#include "server.h"
#include "pch.h"


int main(int argc,char*argv[]){
    if(argc<3){
        std::cerr<<"Usage: ./constella-node <port> <node_id> \n";
        return 1;
    }
    int port=std::stoi(argv[1]);
    std::string node_id=argv[2];
    std::vector<std::string>cluster_nodes={
        "127.0.0.1:6000",
        "127.0.0.1:6001",
        "127.0.0.1:6002"
    };
    Storage storage;
    Server server(port,node_id,cluster_nodes,storage,2,2,1);
    server.start();

    return 0;
}