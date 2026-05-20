#include "storage.h"
#include "server.h"
#include "pch.h"


int main(int argc,char*argv[]){
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    if(argc<3){
        std::cerr<<"Usage: ./constella-node <port> <node_id> \n";
        return 1;
    }
    int port=std::stoi(argv[1]);
    std::string node_id=argv[2];
    std::vector<std::string>cluster_nodes={
        "node1:6000",
        "node2:6001",
        "node3:6002"
    };
    Storage storage;
    Server server(port,node_id,cluster_nodes,storage,2,2,1);
    server.start();

    return 0;
}