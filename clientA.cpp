#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
using namespace std;

#define localhost "127.0.0.1"
#define clienAPort "25296"


// referenced a method for split the string
vector<string> split(string &message){
    vector<string> result;
    const string delimiter = ",";
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


int main(int argc,char*argv[]){
    if(argc<2 || argc >4){
        //incorrect number of input
        cout<<"cannot identify the input argument provide"<<endl;
        return -1;
    }
    // create socket
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1){
        perror("Client socket creation failure");
        return -1;
    }
    // get address info
    struct addrinfo hints, *servinfo;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(localhost,clienAPort,&hints,&servinfo);

    cout<<"The client A is up and running."<<endl;

    if(connect(sockfd,servinfo->ai_addr,servinfo->ai_addrlen) != 0){
        perror("ClientA connection failure ");
        freeaddrinfo(servinfo);
        close(sockfd);
        return -1;
    }
    //create a buffer
    const int buffSize = 8192;
    char buffer[buffSize];

    if(argc == 2){
        string userName = argv[1];
        if(userName !="TXLIST"){
            //here is the check
            sprintf(buffer,"1,%s",argv[1]);
            if(send(sockfd,buffer,strlen(buffer),0) <= 0){
                perror("ClientA sending failure");
                close(sockfd);
                freeaddrinfo(servinfo);
                return -1;
            }
            cout<< userName <<" sent a balance enquiry request to the main server."<<endl;
            memset(buffer,0,sizeof(buffer));
            if(recv(sockfd,buffer,sizeof(buffer),0) <= 0){
                perror("ClientA reciving failure");
                close(sockfd);
                freeaddrinfo(servinfo);
                return -1;
            }
            
            int balance = atoi(buffer);
            if(balance == -1){
                cout<<"Unable to proceed with the transaction as "<<userName<<" is not part of the network."<<endl;
            }else{
                cout<<"The current balance of "<<userName<<" is : "<<balance<<" aliconins."<<endl;
            }
        }else{
            //here is for txlist
            snprintf(buffer,sizeof(buffer),"3");
            if(send(sockfd,buffer,strlen(buffer),0) <= 0){
                perror("ClientA sending failure");
                close(sockfd);
                freeaddrinfo(servinfo);
                return -1;
            }
            cout<<"clientA sent a sorted list request to the main server.\n";
            
        }
    }else if(argc ==3 && strncmp(argv[2],"stats",5) == 0){
        //here is the userstats
        //cout<<"userstatus"<<endl; 
        snprintf(buffer,sizeof(buffer),"4,%s",argv[1]);
        if(send(sockfd,buffer,strlen(buffer),0) <= 0){
            perror("ClientA sending failure");
            close(sockfd);
            freeaddrinfo(servinfo);
            return -1;
        }
        cout<<argv[1]<< " sent a statistics enquiry request to the main server.\n";
        
        char recvBuffer[buffSize];
        if(recv(sockfd,recvBuffer,buffSize,0)<=0){
            perror("TCP recving failure");
            close(sockfd);
            freeaddrinfo(servinfo);
            return -1;
        }

        if(recvBuffer[0]!='#'){
            cout<<argv[1]<<" statistics are the following.:\n";
            cout<<"Rank--Username--NumofTransacions--Total\n";
            cout<<recvBuffer;
        }else{
            cout<<"Unable to proceed the request as "<<argv[1]<<" is not part of the network."<<endl;
        }

    }else if(argc == 4){
        //cout<<"entered with argc = 4";
        //here is the transfer
        snprintf(buffer,sizeof(buffer),"2,%s,%s,%s",argv[1],argv[2],argv[3]);
        if(send(sockfd,buffer,strlen(buffer),0) <= 0){
            perror("ClientA sending failure");
            close(sockfd);
            freeaddrinfo(servinfo);
            return -1;
        }
        cout<< argv[1]<<" has requested to transfer "<<argv[3]<<" conins to "<< argv[2]<<".\n";
        memset(buffer,0,sizeof(buffer));
        if(recv(sockfd,buffer,sizeof(buffer),0) <= 0){
            perror("ClientA reciving failure");
            close(sockfd);
            freeaddrinfo(servinfo);
            return -1;
        }
        string data(buffer);
        vector<string> result= split(data);
        
        if(result[0]== "-1" && result[1] == "-1"){
            cout<<"Unable to proceed with the transaction as "<< argv[1]<<" and "<<argv[2]<<" are not part of the netwrok.\n";
        }else if(result[0] == "-1" && result[1] == "0"){
            cout<<"Unable to proceed with the transaction as "<< argv[1]<<" is not part of the network.\n";
        }else if(result[0] == "0" && result[1] == "-1"){
            cout<<"Unable to proceed with the transaction as "<< argv[2]<<" is not part of the network.\n";
        }else if(result[0] == "-2"){
            cout<<argv[1]<<" was unable to transfer "<<argv[3]<<" alicoins to "<<argv[2]
            <<" because of insufficient balance.\nThe current balance of "<<argv[1]<<" is : "<<result[1]<<" alicoins.\n";
        }else{
            cout<<argv[1]<<" has successfully transfered "<< argv[3]<<" aliconins to "
            <<argv[2]<<".\n" ;
            cout<<"The current balance of "<<argv[1]<<" is : "<<result[0]<<" alicoins.\n";
        }

    }else{
        //nothing happens here 
        cout<<"Invalid Operation!"<<endl;
    }

    close(sockfd);
    freeaddrinfo(servinfo);
    return -1;


}