#pragma once 

#include<string>
#include<map>
#include<vector>

class HashRing{
    public:
        void add_node(const std::string& node_id);
        void remove_node(const std::string& node_id);
        std::string get_node(const std::string& key) const;
        std::vector<std::string>get_all_nodes() const;

    private:
        std::map<uint64_t,std::string>ring_;
        uint64_t hash(const std::string& input) const;
};