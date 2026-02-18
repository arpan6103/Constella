#include "hash_ring.h"


uint64_t HashRing::hash(const std::string& input) const{
    std::hash<std::string>hasher;
    return static_cast<uint64_t>(hasher(input));
}

void HashRing::add_node(const std::string& node_id){
    uint64_t hashed=hash(node_id);
    ring_[hashed]=node_id;
}

void HashRing::remove_node(const std::string& node_id){
    uint64_t hashed=hash(node_id);
    ring_.erase(hashed);
}

std::string HashRing::get_node(const std::string& key) const{
    if(ring_.empty()){
        return "";
    }
    uint64_t hashed_key=hash(key);
    auto it=ring_.lower_bound(hashed_key);
    if(it==ring_.end()){
        it=ring_.begin();
    }
    return it->second;
}

std::vector<std::string> HashRing::get_all_nodes() const{
    std::vector<std::string>nodes;
    for(const auto &it:ring_){
        nodes.push_back(it.second);
    }
    return nodes;
}

std::vector<std::string>HashRing::get_replicas(const std::string& key,int count) const{
    std::vector<std::string>replicas;
    if(ring_.empty() || count<1){
        return replicas;
    }
    uint64_t hashed_key=hash(key);
    auto it=ring_.lower_bound(hashed_key);
    if(it==ring_.end()){
        it=ring_.begin();
    }
    while(replicas.size()<static_cast<size_t>(count)){
        replicas.push_back(it->second);
        it++;
        if(it==ring_.end()){
            it=ring_.begin();
        }
        if(replicas.size()==ring_.size()){
            break;
        }
    }
    return replicas;
}