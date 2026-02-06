#include "storage.h"

void Storage::put(const std::string& key,const std::string& value){
    std::lock_guard<std::mutex>lock(mtx_);
    data_[key]=value;
}

bool Storage::get(const std::string& key,std::string& value){
    std::lock_guard<std::mutex>lock(mtx_);
    auto it=data_.find(key);
    if(it==data_.end()) return false;
    value=it->second;
    return true;
}