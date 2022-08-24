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
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
using namespace std;

#define localhost "127.0.0.1"
#define updAPort "21296"
#define updBPort "22296"
#define updCPort "23296" //last 3 digit
#define udpPort "24296"
#define tcpAPort "25296"
#define tcpBPort "26296"
#define BACKLOG 10
#define buffSize 8192

vector<string> allRecords;

struct stats{
    string userName;
    int transNum;
    int amount;
};

int updSocket(){
    int sockfd;
    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1){
        cout<<"UPD socket creation failure"<<endl; 
        return -1;
    }
    return sockfd;
}

int checkWallet(string userName, string clientName,bool isPrimary){
    //open socket and send info
    int sockfd = updSocket();
    struct addrinfo hints, *servinfo;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    getaddrinfo(localhost,updAPort,&hints,&servinfo);

    int startBalance = 1000;
    bool userExist = false;
    char buffer[buffSize];
    snprintf(buffer,sizeof(buffer),"c,%s", userName.c_str());
    //send and recive info from A
    if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen)== -1){
        perror("UPD sending serverA failure");
    }else{
        if(isPrimary){
            cout<<"The main server sent a request to server A."<<endl;
        }
        memset(buffer,0,sizeof(buffer));
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))== -1){
            perror("UPD receving serverA failure");
        }else{
            //recive successfully
            if(isPrimary){
                cout<<"The main server received transactions from Server A using UDP over prot "<< updAPort<<".\n";
                //cout<<"The message is "<< buffer<<endl;
            }
            if(buffer[0]!='#'){
                startBalance += atoi(buffer);
                userExist = true;
            }
        }

    }
    //send and recive infro from B
    snprintf(buffer,sizeof(buffer),"c,%s", userName.c_str());
    getaddrinfo(localhost,updBPort,&hints,&servinfo);
    if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen)== -1){
        perror("UPD sending serverB failure");
    }else{
        if(isPrimary){
            cout<<"The main server sent a request to server B."<<endl;
        }
        memset(buffer,0,sizeof(buffer));
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))== -1){
            perror("UPD receving serverB failure");
        }else{
            //recive successfully
            if(isPrimary){
                cout<<"The main server received transactions from Server B using UDP over prot "<< updBPort<<".\n";
                //cout<<"The message is "<< buffer<<endl;
            }
            if(buffer[0]!='#'){
                startBalance += atoi(buffer);
                userExist = true;
            }
        }

    }
    
    //send and recive info from C
    snprintf(buffer,sizeof(buffer),"c,%s", userName.c_str());
    getaddrinfo(localhost,updCPort,&hints,&servinfo);
    if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen)== -1){
        perror("UPD sending serverC failure");
    }else{
        if(isPrimary){
            cout<<"The main server sent a request to server C."<<endl;
        }
        memset(buffer,0,sizeof(buffer));
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))== -1){
            perror("UPD receving serverB failure");
        }else{
            //recive successfully
            if(isPrimary){
                cout<<"The main server received transactions from Server C using UDP over prot "<< updCPort<<".\n";
                //cout<<"The message is "<< buffer<<endl;
            }

            if(buffer[0]!='#'){
                startBalance += atoi(buffer);
                userExist = true;
            }
        }

    }
    //close the socket and return
    close(sockfd);
    if(userExist){
        return startBalance; 
    }else{
        return -1;
    }
}

int newSerial(){
    int sockfd = updSocket();
    struct addrinfo hints, *servinfo;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    getaddrinfo(localhost,updAPort,&hints,&servinfo);

    int serial = 0;
    char buffer[buffSize];
    snprintf(buffer,sizeof(buffer),"s");
    if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen)== -1){
        perror("UPD sending serverA failure");
    }else{
        cout<<"The main server sent a request to server A.\n";
        memset(buffer,0,sizeof(buffer));
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))== -1){
            perror("UPD receving serverA failure");
        }else{
            cout<<"The main server received the feedback from server A using UDP over prot "<< updAPort<<".\n";
            serial = max(serial,atoi(buffer));
        }

    }
    //here forB
    snprintf(buffer,sizeof(buffer),"s");
    getaddrinfo(localhost,updBPort,&hints,&servinfo);
    if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen)== -1){
        perror("UPD sending serverB failure");
    }else{
        cout<<"The main server sent a request to server B.\n";
        memset(buffer,0,sizeof(buffer));
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))== -1){
            perror("UPD receving serverB failure");
        }else{
            cout<<"The main server received the feedback from server B using UDP over prot "<< updBPort<<".\n";
            serial = max(serial,atoi(buffer));
        }

    }
    // here for C
    snprintf(buffer,sizeof(buffer),"s");
    getaddrinfo(localhost,updCPort,&hints,&servinfo);
    if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen)== -1){
        perror("UPD sending serverC failure");
    }else{
        cout<<"The main server sent a request to server C.\n";
        memset(buffer,0,sizeof(buffer));
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))== -1){
            perror("UPD receving serverC failure");
        }else{
            cout<<"The main server received the feedback from server C using UDP over prot "<< updBPort<<".\n";
            serial = max(serial,atoi(buffer));
        }
    }
    close(sockfd);
    return serial;

}


//TXCOINS function
vector<int> txCoins(string sender, string reciever, int amount,string clientName){
    vector<int> result;
    int senderBalance = checkWallet(sender,clientName,false);
    int reciverBalance = checkWallet(reciever,clientName,false);

    //probably returned too early
    if(senderBalance == -1 && reciverBalance ==-1){
        result.push_back(-1);
        result.push_back(-1);
        return result;
    }
    if(senderBalance == -1){
        result.push_back(-1);
        result.push_back(0);
        return result;
    }
    if(reciverBalance == -1){
        result.push_back(0);
        result.push_back(-1);
        return result;
    }
    if(senderBalance < amount){
        result.push_back(-2);
        result.push_back(senderBalance);
        return result;
    }   


    int serial = newSerial()+1;
    int sockfd = updSocket();
    struct addrinfo hints, *servinfo;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    char buffer[buffSize];

    //now we choose a server to write the new transaction
    srand((unsigned)time(NULL));
    int ranNum = rand()%3;
    if(ranNum == 0){
        getaddrinfo(localhost,updAPort,&hints,&servinfo);
    }else if(ranNum == 1){
        getaddrinfo(localhost,updBPort,&hints,&servinfo);
    }else{
        getaddrinfo(localhost,updCPort,&hints,&servinfo);
    }
    snprintf(buffer,sizeof(buffer),"w,%d %s %s %d",serial,sender.c_str(),reciever.c_str(),amount);

    if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen)== -1){
        perror("UPD write request sending server failure");
    }else{
        memset(buffer,0,sizeof(buffer));
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))== -1){
            perror("UPD  write request reciving failure");
        }else{
            //cout<<"here is the response from the writing "<<buffer<<endl;
            senderBalance = checkWallet(sender,clientName,false);
        }
    }
    result.push_back(senderBalance);
    result.push_back(0);
    // cout<< "The most up to date serial number is"<<serial<<endl;
    //here it indicates that transaction can be done 
    close(sockfd);
    return result;


}



// referenced a method for split the string
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

void getList(string portNum){
    int sockfd = updSocket();
    struct addrinfo hints, *servinfo;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    char buffer[buffSize];
    snprintf(buffer,buffSize,"l");
    getaddrinfo(localhost,portNum.c_str(),&hints,&servinfo);
    
    if(sendto(sockfd,buffer,strlen(buffer),0,servinfo->ai_addr,servinfo->ai_addrlen) == -1){
        perror("getList request sedning Failure");
    }else{
        memset(buffer,0,buffSize);
        if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))==-1){
            perror("getList first record receving Failure");
        }else{
            string data(buffer);
            vector<string> first =split(data,",");
            int n = atoi(first[0].c_str());
            allRecords.push_back(first[1]);
            //cout<<"The current record received is :"<< buffer<<endl;
            for(int i=1;i<n;i++){
                memset(buffer,0,sizeof(buffer));
                if(recvfrom(sockfd,buffer,sizeof(buffer),0,servinfo->ai_addr,&(servinfo->ai_addrlen))==-1){
                    perror("getList first record receving Failure");
                }else{
                    string currData(buffer);
                    vector<string> curr = split(currData,",");
                    allRecords.push_back(curr[1]);
                    //cout<<"The current record received is :"<< curr[1]<<endl;
                }
                
            }

        }

    }
    close(sockfd);
}


bool compare(string recordA, string recordB){
    vector<string> argsA = split(recordA," ");
    vector<string> argsB = split(recordB," ");
    int a = atoi(argsA[0].c_str());
    int b = atoi(argsB[0].c_str());
    return a<b;
}

void txList(){
    allRecords.clear();
    getList(updAPort);
    getList(updBPort);
    getList(updCPort);
    sort(allRecords.begin(),allRecords.end(),compare);

    ofstream output;
    output.open("alichain.txt",ios::out);
    for(int i =0 ;i<allRecords.size();i++){
        output<<allRecords[i]<<endl;
    }
    output.close();
}

bool compare2(struct stats a, struct stats b){
    return a.transNum>b.transNum;
}

void status(string userName,int childFd){
    allRecords.clear();
    getList(updAPort);
    getList(updBPort);
    getList(updCPort);
    map<string, struct stats>infoMap;
    for(int i = 0; i<allRecords.size();i++){
        vector<string> currArgs = split(allRecords[i]," ");
        //1 sender 2 reciver 3 amout;
        if(currArgs[1] == userName || currArgs[2] == userName){
            //the record is relevent
            int currAmount = atoi(currArgs[3].c_str());
            if(currArgs[1] == userName){
                //case sneder
                if(infoMap.count(currArgs[2])){
                    infoMap[currArgs[2]].amount -= currAmount;
                    infoMap[currArgs[2]].transNum++;
                }else{
                    struct stats curr;
                    curr.userName = currArgs[2];
                    curr.transNum = 1;
                    curr.amount = -1*currAmount;
                    infoMap.insert(pair<string,struct stats>(currArgs[2],curr));
                }
            }else{
                //case reciver
                if(infoMap.count(currArgs[1])){
                    infoMap[currArgs[1]].amount += currAmount;
                    infoMap[currArgs[1]].transNum++;
                }else{
                    struct stats curr;
                    curr.userName = currArgs[1];
                    curr.transNum = 1;
                    curr.amount = currAmount;
                    infoMap.insert(pair<string,struct stats>(currArgs[1],curr));
                }
            }

        }

    }



    // here we construct the array for sorting and sending. 
    vector<struct stats> statRecords; 
    map<string,struct stats>::iterator itor;
    for(itor = infoMap.begin();itor != infoMap.end();itor++){
        // stringstream curr;
        // curr<<itor->second.userName<<" "<<itor->second.transNum<<" "<< itor->second.amount<<endl;
        // statRecords.push_back(curr.str());
        // cout<<curr.str();
        struct stats st;
        st.transNum = (itor->second).transNum;
        st.userName = (itor->second).userName;
        st.amount = (itor->second).amount;
        statRecords.push_back(st);
    }
    


    // the we sort it;

    char buffer[buffSize];
    if(statRecords.size() == 0){
        snprintf(buffer,buffSize,"#");
        if(send(childFd,buffer,strlen(buffer),0)<=0){
            perror("TCP sending failure in status request");
        }
        return;
    }

    sort(statRecords.begin(),statRecords.end(),compare2);
    for(int i =0;i<statRecords.size();i++){
        //cout<<statRecords[i].userName<< " "<< statRecords[i].transNum<<" "<< statRecords[i].amount <<endl;
        snprintf(buffer,buffSize,"%d  %s  %d  %d\n",i+1,statRecords[i].userName.c_str(),statRecords[i].transNum,statRecords[i].amount);
        if(send(childFd,buffer,strlen(buffer),0)<=0){
            perror("TCP sending failure in status request");
        }
    }


    

}


int reqHandler(char* message, string clientName,int childFd){
    // here we don't need the client address 
    string data(message);
    vector<string> args = split(data,",");
    string curPort = clientName == "A"? tcpAPort:tcpBPort;

    //cout<<"I recived information: "<<message<<endl;

    if(args[0] == "1"){
        //check wallet
        cout<<"The main server received input="<<args[1]
        << " from the client using TCP over port "<< curPort<<"." <<endl;
        // int balance = checkWallet(args[1],clientName,true);
        int balance = checkWallet(args[1],clientName,true);
        snprintf(message,buffSize,"%d",balance);
        return 1;
    }else if(args[0] == "2"){
        cout<<"The main server received from "<<args[1]<<" to transfer "<<args[3]
        <<" coins to "<<args[2]<<" using TCP over port "<< curPort <<"."<<endl;
        //tansfer the amout.
        vector<int>curr = txCoins(args[1],args[2],atoi(args[3].c_str()),clientName);
        //we are testing here nothing to serious
        snprintf(message,buffSize,"%d,%d",curr[0],curr[1]);
        return 2;

    }else if(args[0] == "3"){
        //TXLIST request
        txList();
        //need to comment out
        snprintf(message,buffSize,"txList runned in the serverM");
        return 3;
    }else if(args[0] == "4"){
        //cout<<"req handler ran\n";
        status(args[1],childFd);
        return 4;
    }else{
        cout<<"operation invalid"<<endl;
    }
    
    return -1;

}

int servA, servB;
int main(){
    int yes =1;
    
    if((servA = socket(AF_INET,SOCK_STREAM,0)) == -1){
        perror("socketA creation failure");
        return -1;
    }

    if((servB = socket(AF_INET,SOCK_STREAM,0)) == -1){
        perror("socketB creation failure");
        close(servA);
        return -1;
    }

    if(setsockopt(servA,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))== -1){
        perror("socketA setsockopt");
        return -1;
    }

    if(setsockopt(servB,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))== -1){
        perror("socketA setsockopt");
        return -1;
    }
    // get the addressinfo 
    struct addrinfo hints,*serveinfo;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(localhost,tcpAPort,&hints,&serveinfo);
    // bind A
    if(bind(servA,serveinfo->ai_addr,serveinfo->ai_addrlen) != 0 ){
        perror("scoketA binding failure");
        close(servA);
        return -1;
    }

    getaddrinfo(localhost,tcpBPort,&hints,&serveinfo);
    // bind B
    if(bind(servB,serveinfo->ai_addr,serveinfo->ai_addrlen) != 0 ){
        perror("scoketB binding failure");
        close(servA);
        close(servB);
        return -1;
    }
    freeaddrinfo(serveinfo);
    //listsen on the serverPort A and B
    if(listen(servA,BACKLOG) < 0){
        perror("TCPA listening failure");
        close(servA);
        close(servB);
        return -1;
    }

    if(listen(servB,BACKLOG)<0){
        perror("TCPB listening failure");
        close(servA);
        close(servB);
        return -1;
    }

    

    cout<<"The main server is up and running"<<endl;
    // below I used the select function to deal with client multiplexing. code was adapted from Beej's Guide page 106
    fd_set master;
    int fdmax;
    FD_ZERO(&master);
    FD_SET(servA,&master);
    FD_SET(servB,&master);
    fdmax = max(servA,servB); //keep track of the biggest file descriptor;

    //create veriable for accepting sending and receving
    struct sockaddr_in clientAddr, cleintBddr;
    socklen_t addrSize;
    char bufferA[buffSize],bufferB[buffSize];


    
    while(true){
        //accept an clientA
        fd_set readfds;
        FD_ZERO(&readfds);
        readfds = master;

        if(select(fdmax+1,&readfds,NULL,NULL,NULL) == -1){
            perror("selecting client failure");
        }

        if(FD_ISSET(servA,&readfds)){ // A has a request
            int childA = accept(servA,(struct sockaddr *)&clientAddr,&addrSize);
            if(childA <0){
                perror("TCPA accept failure");
                continue;
            }
            memset(bufferA,0,sizeof(bufferA));
            int receiveA = recv(childA,bufferA,sizeof(bufferA),0);
            if(receiveA <=0){
                perror("TCPA receive no information or failure");
                continue;
            }
            string client = "A";
            // i don't believe we need this client Addr to go in since we are hardcoding childsocket already.
            int opNum = reqHandler(bufferA,client,childA);

            if(opNum != 4){
                if(send(childA,bufferA,sizeof(bufferA),0)<0){
                    perror("TCPA sending failure");
                }else{
                    //server output according to the upNum
                    if(opNum ==1){
                        cout<<"The main server sent the current balance to client A."<<endl;
                    }else if(opNum == 2){
                        cout<<"The main server sent the result of the transaction to client A."<<endl;
                    }
                }
            }
            close(childA);
        }
        

        if(FD_ISSET(servB,&readfds)){ //B has a request
            int childB = accept(servB,(struct sockaddr *)&cleintBddr,&addrSize);
            if(childB <0){
                perror("TCPB accept failure");
                continue;
            }
            memset(bufferB,0,sizeof(bufferB));
            int reciveB = recv(childB,bufferB,sizeof(bufferB),0);
            if(reciveB <=0){
                perror("TCPB receive no information or failure");
                continue;
            }
            string client = "B";
            int opNum = reqHandler(bufferB,client,childB);

            if(opNum != 4){
                if(send(childB,bufferB,sizeof(bufferB),0)<0){
                    perror("TCPB sending failure");
                }else{

                    if(opNum ==1){
                        cout<<"The main server sent the current balance to client B."<<endl;
                    }else if(opNum == 2){
                        cout<<"The main server sent the result of the transaction to client B."<<endl;
                    }
                }
            }
            close(childB);

        }


    }
    
    close(servA);
    close(servB);
    return -1;
}