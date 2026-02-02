#include "storage.h"

void Storage::put(const string& key,const string& value){
    lock_guard<mutex>lock(mtx_);
    data_[key]=value;
}

bool Storage::get(const string& key,string& value){
    lock_guard<mutex>lock(mtx_);
    auto it=data_.find(key);
    if(it==data_.end()) return false;
    value=it->second;
    return true;
}