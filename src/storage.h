#pragma once
#include<unordered_map>
#include<string>
#include<mutex>
using namespace std;

class Storage{
    public:
    void put(const string& key,const string& value);
    bool get(const string& key,string& value);

    private:
    unordered_map<string,string>data_;
    mutex mtx_;
};