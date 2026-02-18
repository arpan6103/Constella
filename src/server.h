#pragma once

#include "storage.h"
#include "hash_ring.h"

class Server{
    public:
        Server(int port,const std::string& node_id,const std::vector<std::string>& cluster_nodes,Storage& storage,int replication_factor=2);
        void start();

    private:
        int port_;
        int server_fd_{-1};
        std::string node_id_;
        HashRing ring_;
        int replication_factor_;
        Storage& storage_;
        
        void handle_client(int client_fd);
        std::string forward_request(const std::string& owner, const std::string& request);

};