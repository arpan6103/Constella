#include "storage.h"
#include "server.h"
#include "pch.h"


int main(int argc,char*argv[]){
    if(argc<3){
        std::cerr<<"Usage: ./constella-node <port> <node_id> <cluster_nodes...>\n";
        return 1;
    }
    int port=std::stoi(argv[1]);
    std::string node_id=argv[2];
    std::vector<std::string>cluster_nodes;
    for(int i=2;i<argc;i++){
        cluster_nodes.push_back(argv[i]);
    }
    Storage storage;
    Server server(port,node_id,cluster_nodes,storage);
    server.start();

    return 0;
}