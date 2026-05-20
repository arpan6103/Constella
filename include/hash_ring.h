#pragma once 

#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<unordered_set>

class HashRing{
    public:
        HashRing(int virtual_nodes=150);
        void add_node(const std::string& node_id);
        void remove_node(const std::string& node_id);
        std::string get_node(const std::string& key) const;
        std::vector<std::string>get_replicas(const std::string& key,int count) const;
        std::vector<std::string>get_all_nodes() const;

    private:
        int virtual_nodes_;
        std::map<uint64_t,std::string>ring_;
        uint64_t hash(const std::string& input) const;
};