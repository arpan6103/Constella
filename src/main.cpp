#include "storage.h"
#include "server.h"
#include "pch.h"


int main(int argc,char*argv[]){
    if(argc!=2){
        std::cerr<<"Usage: ./constella-node <port>\n";
        return 1;
    }
    int port=std::stoi(argv[1]);
    Storage storage;
    Server server(port,storage);

    server.start();

    return 0;
}