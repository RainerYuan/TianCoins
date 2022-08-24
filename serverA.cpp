#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

#define localhost "127.0.0.1"
#define filePath "block1.txt"
#define updAPort "21296" //last 3 digit
#define updMport "24296"
#define buffSize 8192

int sockfd;
struct addrinfo hints,*servinfo;

vector<string> split(string &message,string delimiter){
    vector<string> result;
    //const string delimiter = ",";
    int len = message.length();
    int i = 0, k= 0;
    while(i<len){
        int j = 0;
        while(i+j<len && j<1 && message[i+j] == delimiter[j]){
            j++;
        }
        if(j == 1){
            result.push_back(message.substr(k,i-k));
            i+=1;
            k=i;
        }else{
            i++;
        }
    }

    result.push_back(message.substr(k,i-k));
    return result;
}

string checkWallet(string userName){
    int balance = 0;
    bool userFound = false;
    ifstream curBlock;
    curBlock.open(filePath);
    string line;
    while(getline(curBlock,line)){
        vector<string> records = split(line," ");
        if(records[1] == userName){
            // it is sender 
            userFound = true;
            balance -= atoi(records[3].c_str());
        }else if(records[2] == userName){
            // it is receiver
            userFound = true;
            balance += atoi(records[3].c_str());
        }
    }
    
    if(userFound){
        stringstream ans;
        ans << balance;
        return ans.str();
    }
    
    return "#";
    
}

int getSeriralNumber(){
    int serial = 0;
    ifstream curBlock;
    curBlock.open(filePath);
    string line;

    while(getline(curBlock,line)){
        vector<string> records = split(line," ");
        int curNum = atoi(records[0].c_str());
        if(curNum>serial){
            serial = curNum;
        }
    }
    curBlock.close();
    return serial;

}

vector<string> allRecords(){
    ifstream curBlock;
    curBlock.open(filePath);
    string line;
    vector<string> result;
    while(getline(curBlock,line)){
        if(!line.empty()){
            result.push_back(line);
        }
    }
    curBlock.close();
    return result;
}

int reqHandler(char* message){
    string data(message);
    vector<string> args = split(data,",");
    if(args[0] == "c"){
        //check wallet
        string balance = checkWallet(args[1]);
        snprintf(message,sizeof(message),"%s",balance.c_str());
        return 1;
    }else if(args[0] =="s"){
        //get serial number
        int newSerial = getSeriralNumber();
        snprintf(message,sizeof(message),"%d",newSerial); 
        return 2;
    }else if(args[0] == "w"){
        //write the log
        ofstream output;
        output.open(filePath,ios::ios_base::app);
        output<<args[1]<<endl;
        output.close();
        snprintf(message,buffSize,"new entry successfully saved in A");
        return 3;
    }else if(args[0] == "l"){
        //for list of records
        char buffer[buffSize];
        vector<string> records = allRecords();
        int numRecords = records.size();
        for(int i = 0; i<numRecords;i++){
            sprintf(buffer,"%d,%s",numRecords,records[i].c_str());
            if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen)==-1){
                perror("Txlist sending records failure");
            }
        }
        cout<<"The ServerA finished sending the response to the Main Server\n";
        return 4;

    }

    return -1;
}


int main(){
    // creating and binding scoket reference the Beej's Guide
    // first we create the socket
    sockfd = socket(AF_INET,SOCK_DGRAM,0);

    if(sockfd == -1){
        perror("UDP socket creation failure");
        return -1;
    }
    // variables for getting address info   
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    getaddrinfo(localhost,updAPort,&hints,&servinfo);

    if(bind(sockfd,servinfo->ai_addr,servinfo->ai_addrlen) != 0){
        perror("UDP socket binding failure");
        close(sockfd);
        return -1;
    }
    // get the addressinfor of m
    getaddrinfo(localhost,updMport,&hints,&servinfo);
    cout<<"The ServerA is up and running using UDP on port "<<updAPort<<".\n";
    char buffer[buffSize];
    while(true){
        memset(buffer,0,sizeof(buffer));
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))==0){
            perror("reciving eorror happend");
            return -1;
        }else{
            cout<<"The ServerA received a request from the Main Server"<<endl;
        }

        int opNum = reqHandler(buffer);
        //cout<< "here is up and running opNum";
        if(opNum != 4){
            //if not the last two send directly
            if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen) == -1){
                perror("sending back error");
                return -1;
            }else{
                cout<<"The ServerA finished sending the response to the Main Server\n";
            }
        }


    }


    close(sockfd);
    return -1;
}