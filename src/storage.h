#pragma once
#include<string>
#include<mutex>
#include<unordered_map>
class Storage{
    public:
    void put(const std::string& key,const std::string& value);
    bool get(const std::string& key,std::string& value);

    private:
    std::unordered_map<std::string,std::string>data_;
    std::mutex mtx_;
};