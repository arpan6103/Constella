#pragma once

#include "storage.h"
#include "hash_ring.h"

#include<unordered_set>
#include<mutex>

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

        std::unordered_set<std::string>processed_requests_;
        std::mutex processed_mutex_;
        std::atomic<uint64_t>request_counter_{0};

        void handle_client(int client_fd);
        std::string forward_request(const std::string& owner, const std::string& request);

        std::string generate_request_id();

};