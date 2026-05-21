#include "storage.h"
#include "pch.h"

Storage::Storage(const std::string& wal_path):wal_path_(wal_path){
    replay_wal();
    wal_file_.open(wal_path_,std::ios::app);
    if(!wal_file_.is_open()){
        std::cerr<<"Failed to open WAL file: "<<wal_path_<<"\n";
    }
}

void Storage::replay_wal(){
    std::ifstream file(wal_path_);
    if(!file.is_open()){
        return;
    }
    std::string line;
    int count=0;
    while(std::getline(file,line)){
        std::istringstream ss(line);
        std::string op,key,value;
        ss>>op>>key;
        std::getline(ss,value);
        if(!value.empty() && value[0]==' '){
            value=value.substr(1);
        }
        if(op=="PUT" && !key.empty()){
            data_[key]=value;
            count++;
        }
    }
    std::cout<<"WAL replayed "<<count<<" entries from "<<wal_path_<<"\n";
}

void Storage::put(const std::string& key,const std::string& value){
    std::lock_guard<std::mutex>lock(mtx_);
    data_[key]=value;
    wal_file_<<"PUT "<<key<<" "<<value<<"\n";
    wal_file_.flush(); 
}

bool Storage::get(const std::string& key,std::string& value){
    std::lock_guard<std::mutex>lock(mtx_);
    auto it=data_.find(key);
    if(it==data_.end()) return false;
    value=it->second;
    return true;
}