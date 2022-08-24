# EE450_Spring2022_Programming_ProjectA
Author Name: Tianyuan Yuan
Student ID:6353048296

c: I have implemented all functionalities including the bonus "stats" inquriy functionality. 

d: There are total 6 cpp code files, one makefile, and one read me file. 
serverM.cpp contains code for the serverM mentioned in the assignment, it communicate with 3 backend servers, perform the required functions and send result to client if neccessary. 
serverA.cpp, serverB.cpp, serverC.cpp contain code to extract information from the block text file and send to serverM accordingly. 
clientA.cpp and clientB.cpp contains code that recoginze user input, send corresponding request to serverM and also display the information recived back. 

e: Message Exchange

clientA,B to serverM:
check wallet: "1,username"
TXCOINS:"2,sender,reciver,amount"
TXLIST:"3"
stats:"4,username"

serverM to serverA,B,C:
check user balance: "c,username"
get highest serial number in server:"s"
write current transaction to the block file: "w,transactionMessage"
request a list of all transaction from the server:"l";

serverA,B,C to serverM:
check user balance: "amount" or "#" if no record for the username found
gethighest serial number: "highestSerialNumber"
write current transaction to the block file: "new entry successfully saved in server"
request a list of all transactions: "transactionsRecord"

serverM to clientA,B
check wallet:"totalBalance", "-1" if user doesn't exist
TXCOINS:"senderBlance, reciverBlance" if one of them does not exist the blance will be -1;
stats:a sorted list of all the stats records;

g:As long As the userInput format is correct, the project should not fail.
the serverM will not boot if error occur in socket creation, binding, listensing, accepting, selecting,sending and receving

h: ALl the cpp code file contains a split function which split up a string by a specifc delimeter and return a vector of string for each part. I referened this function online at http://www.zedwood.com/article/cpp-explode-function

I also reused code on beej's guide page 106 for the usage of select. All the socket programming function calls are also came from this book. However, I didn't directly copy any block of code from this book, the writing style and naming for variables in those function call can be very similar.

The libraries I used that is not included in the guide are listed below:

sstream, algorithm, iostream, fstream, map, vector.