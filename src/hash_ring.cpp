#include "hash_ring.h"


uint64_t HashRing::hash(const std::string& input) const {
    uint64_t hash = 14695981039346656037ULL; // FNV offset basis
    for (unsigned char c : input) {
        hash ^= c;
        hash *= 1099511628211ULL;            // FNV prime
    }
    return hash;
}

HashRing::HashRing(int virtual_nodes):virtual_nodes_(virtual_nodes){}

void HashRing::add_node(const std::string& node_id){
    for(int i=0;i<virtual_nodes_;i++){
        std::string vnode_key=node_id+"#"+std::to_string(i);
        uint64_t hashed=hash(vnode_key);
        ring_[hashed]=node_id;
    }
}

void HashRing::remove_node(const std::string& node_id){
    for(int i=0;i<virtual_nodes_;i++){
        std::string vnode_key=node_id+"#"+std::to_string(i);
        uint64_t hashed=hash(vnode_key);
        ring_.erase(hashed);
    }
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
    std::unordered_set<std::string>seen;
    std::vector<std::string>nodes;
    for(const auto &it:ring_){
        if(seen.insert(it.second).second){
            nodes.push_back(it.second);
        }
    }
    return nodes;
}

std::vector<std::string>HashRing::get_replicas(const std::string& key,int count) const{
    std::vector<std::string>replicas;
    if(ring_.empty() || count<1){
        return replicas;
    }
    size_t total_physical = ring_.size() / virtual_nodes_;
    uint64_t hashed_key=hash(key);
    auto it=ring_.lower_bound(hashed_key);
    if(it==ring_.end()){
        it=ring_.begin();
    }
    std::unordered_set<std::string>seen;
    while(replicas.size() < static_cast<size_t>(count) && seen.size() < total_physical) {
        if(seen.find(it->second) == seen.end()) {
            seen.insert(it->second);
            replicas.push_back(it->second);
        }
        ++it;
        if(it == ring_.end()) it = ring_.begin();
    }
    return replicas;
}