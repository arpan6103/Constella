#pragma once
#include<string>
#include<mutex>
#include<unordered_map>
#include<fstream>
class Storage{
    public:
    Storage(const std::string& wal_path);
    void put(const std::string& key,const std::string& value);
    bool get(const std::string& key,std::string& value);

    private:
    std::unordered_map<std::string,std::string>data_;
    std::mutex mtx_;
    std::ofstream wal_file_;
    std::string wal_path_;

    void replay_wal();
};