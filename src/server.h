#pragma once
#include "storage.h"

class Server{
    public:
        Server(int port,Storage& storage);
        void start();

    private:
        int port_;
        int server_fd_;
        Storage& storage_;
        
        void handle_client(int client_fd);
};