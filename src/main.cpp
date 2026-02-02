#include<iostream>
#include<sstream>
#include "storage.h"

using namespace std;

int main(){
    Storage storage;
    string line;

    cout<<"Constella node 1 started"<<endl;
    cout<<"Commands:  PUT <key> <value> || GET <key> || EXIT"<<endl;

    while(true){
        cout<<"> ";
        if(!getline(cin,line)) break;
        stringstream ss(line);
        string cmd;
        ss>>cmd;
        if(cmd=="PUT"){
            string key,value;
            ss>>key>>value;
            if(key.empty() || value.empty()){
                cout<<"ERROR: PUT <KEY> <VALUE>"<<endl;
                continue;
            }
            storage.put(key,value);
            cout<<"OK"<<endl;
        }
        else if(cmd=="GET"){
            string key,value;
            ss>>key;
            if(key.empty()){
                cout<<"ERROR: GET <KEY>"<<endl;
                continue;
            }
            if(storage.get(key,value)){
                cout<<"Value Found: "<<value<<endl;
            }
            else{
                cout<<"Value Not Found"<<endl;
            }
        }
        else if(cmd=="EXIT"){
            break;
        }
        else{
            cout<<"Command Not Found"<<endl;
        }
    }

    return 0;
}