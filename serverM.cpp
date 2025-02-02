/*
Author: Rajnandini Thopte

serverM.cpp

This code is for the main server that handles requests from clients and also manages backend Servers A & B that store the users
and their availabilities. It communicates with the client via TCP and with both backend servers via UDP. Once the main server
receives the request from client, it decides which backend server the participants’ availability is stored in and sends a request
to the responsible backend server for the time intervals that works for all participants belonging to this backend server.
After the backend servers sends their respective intersection result back to the main server, it runs an algorithm
to get the final time slots that works for all participants, and sends the result back to the client.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <algorithm>

#define SERVER_TCP_PORT 24463
#define SERVERM_UDP 23463
#define SERVER_A 21463
#define SERVER_B 22463
#define BUFFER_SIZE 10000
#define LOCALHOST "127.0.0.1"
#define FAIL -1
#define MAX_USERNAME_LENGTH 20
#define BACKLOG 10 // max number of incoming connections allowed

int sockfd_UDP;
int serverM_clientFD;// parent TCP socket
int childSocketFD;// child TCP socket
struct sockaddr_in serverM_client_addr; // serverM
socklen_t len = 0;
struct sockaddr_in destClient_addr; //parent listening socket
struct sockaddr_in recvaddr;
struct sockaddr_in sendaddrA;
struct sockaddr_in recvaddrB;
struct sockaddr_in sendaddrB;

using namespace std;

// Repurposed from Beej’s socket programming tutorial
// To create the TCP socket for communication with client
void createTCPSocket()
{
    serverM_clientFD = socket(AF_INET, SOCK_STREAM, 0);

    if (serverM_clientFD == FAIL)
    {
        perror("[ERROR] Server M failed to create TCP Socket for Client");
        exit(1);
    }

    // To avoid 'port already in use' error message
    int yes = 1;
    if (setsockopt(serverM_clientFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    // Initialize IP address, port number
    memset(&serverM_client_addr, 0, sizeof(serverM_client_addr));
    serverM_client_addr.sin_family = AF_INET;
    serverM_client_addr.sin_addr.s_addr = inet_addr(LOCALHOST); // Host IP address
    serverM_client_addr.sin_port = htons(SERVER_TCP_PORT);      // Server TCP port

    // Bind socket
    if (::bind(serverM_clientFD, (struct sockaddr *)&serverM_client_addr, sizeof(serverM_client_addr)) == FAIL)
    {
        perror("[ERROR] Server M failed to bind TCP socket");
        exit(1);
    }
}

// Repurposed from Beej’s socket programming tutorial
// Listen for incoming client requests for the TCP parent socket
void listenToClient()
{
    if (listen(serverM_clientFD, BACKLOG) == FAIL)
    {
        perror("[ERROR] Server M failed to listen to client socket");
        exit(1);
    }
}

// Repurposed from Beej’s socket programming tutorial
void createUDPSocket()
{
    sockfd_UDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_UDP == FAIL)
    {
        perror("[ERROR] Server M failed to create UDP socket.");
        exit(1);
    }

    // Initialize IP address, port number
    memset(&recvaddr, 0, sizeof(recvaddr));
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_addr.s_addr = inet_addr(LOCALHOST);
    recvaddr.sin_port = htons(SERVERM_UDP);
}

// Repurposed from Beej’s socket programming tutorial
//Initialize to backend server A
void initializeToServerA()
{
    memset(&sendaddrA, 0, sizeof(sendaddrA));
    sendaddrA.sin_family = AF_INET;
    sendaddrA.sin_addr.s_addr = inet_addr(LOCALHOST);
    sendaddrA.sin_port = htons(SERVER_A);
}

// Repurposed from Beej’s socket programming tutorial
//Initialize to backend server B
void initializeToServerB()
{
    memset(&sendaddrB, 0, sizeof(sendaddrB));
    sendaddrB.sin_family = AF_INET;
    sendaddrB.sin_addr.s_addr = inet_addr(LOCALHOST);
    sendaddrB.sin_port = htons(SERVER_B);
}

/*
This function receives the sublist of usernames that is to be parsed and processed. This is part of Phase 2 where we check which usernames
belong to which backend server and send the usernames to that respective server for further processing.
*/
void Phase2_sendServer_A_B(vector<string> subListToProcess, struct sockaddr_in sendaddrA_B)
{
    string sublist_str;
    for (const auto &username : subListToProcess)
    {
        sublist_str += username + " ";
    }

    int bytes_sent = sendto(sockfd_UDP, sublist_str.c_str(), sublist_str.length(), 0, (struct sockaddr *)&sendaddrA_B, sizeof(sendaddrA_B));
    if (bytes_sent < 0)
    {
        perror("Error sending data to backend server ");
    }


}

/*
In this function we parse the intersection result that the Main Server receives either from server A or B.
*/
vector<pair<int, int>> ParseIntervals(const char *buffer)
{
    vector<pair<int, int>> intervals;
    char *avail = strtok((char *)buffer, "[]");
    while (avail != NULL)
    {
        string str(avail);
        if (str != "," && str != "" && str != " ")
        {
            int comma_pos = str.find(",");
            int start = stoi(str.substr(0, comma_pos));
            int end = stoi(str.substr(comma_pos + 1));
            intervals.emplace_back(start, end);
        }
        avail = strtok(NULL, "[]");
    }
    return intervals;
}


//This is the algorithm that calculates the common time availability among multiple users.

vector<pair<int, int>> CommonTimeAvailability(vector<pair<int, int>> intervals1, vector<pair<int, int>> intervals2)
{
    vector<pair<int, int>> commonIntervals;
    int i = 0, j = 0;
    while (i < intervals1.size() && j < intervals2.size())
    {
        // Check if the intervals overlap
        if (intervals1[i].second < intervals2[j].first)
        {
            i++;
        }
        else if (intervals2[j].second < intervals1[i].first)
        {
            j++;
        }
        else
        {
            int start = max(intervals1[i].first, intervals2[j].first);
            int end = min(intervals1[i].second, intervals2[j].second);
            if (start < end) // check if it is a valid interval
            {
                commonIntervals.emplace_back(start, end);
            }
            if (intervals1[i].second < intervals2[j].second)
            {
                i++;
            }
            else
            {
                j++;
            }
        }
    }
    return commonIntervals;
}

int main()
{
    char bufferA[BUFFER_SIZE];
    char bufferB[BUFFER_SIZE];

    //Creating TCP socket
    createTCPSocket();
    // Start listening to client requests
    listenToClient();
    // Getting socket name
    socklen_t len = sizeof(serverM_client_addr);
    if (getsockname(serverM_clientFD, (struct sockaddr *)&serverM_client_addr, &len) == -1)
    {
        std::cerr << "Failed to get socket name" << std::endl;
        close(serverM_clientFD);
        return 1;
    }

    //std::cout << "DEBUG:: Server is listening on port " << ntohs(serverM_client_addr.sin_port) << std::endl;

    createUDPSocket();
    cout << "Main Server M is up and running." << endl;
    //bind socket
    if (::bind(sockfd_UDP, (sockaddr *)&recvaddr, sizeof(recvaddr)) < 0)
    {
        perror("bind error");
        return 1;
    }

    // PHASE 1
    //-------Server A-------
    //Initializing to server A
    initializeToServerA();
    memset(bufferA, 0, sizeof(bufferA));
    socklen_t sendaddrA_len = sizeof(sendaddrA);
    //Receive usernames only of a.txt from server A.
    int phase1_recv_A = recvfrom(sockfd_UDP, bufferA, sizeof(bufferA), 0, (struct sockaddr *)&sendaddrA, &sendaddrA_len);
    if (phase1_recv_A == FAIL)
    {
        perror("[ERROR] Server M failed to receive data from server A.");
        exit(1);
    }
    //Terminate received data with \0
    bufferA[phase1_recv_A] = '\0';

    // To parse comma separated value
    //Defining a Map to store usernames received from A
    map<string, bool> serverAMap;
    // Loop through each key and add it to the map with a value of true
    string delimiter = ",";
    size_t pos = 0;
    string tokenA;
    string keys(bufferA);
    while ((pos = keys.find(delimiter)) != string::npos)
    {
        tokenA = keys.substr(0, pos);
        serverAMap[tokenA] = true;
        keys.erase(0, pos + delimiter.length());
    }
    cout << "Main Server received the username list from server A using UDP over port " << SERVERM_UDP <<"." << endl;

    //-------Server B-------
     //Initializing to server A
    initializeToServerB();
    socklen_t sendaddrB_len = sizeof(sendaddrB);
    memset(bufferB, 0, sizeof(bufferB));
    //Receive usernames only of b.txt from server B.
    int phase1_recv_B = recvfrom(sockfd_UDP, bufferB, sizeof(bufferB), 0,(struct sockaddr *)&sendaddrB, &sendaddrB_len);
    if (phase1_recv_B== FAIL)
    {
        perror("[ERROR] Server M failed to receive data from server B.");
        return 1;
    }
    //Terminate received data with \0
    bufferB[phase1_recv_B] = '\0';

    // To parse comma separated value
    //Defining a Map to store usernames received from B
    map<string, bool> serverBMap;
    // Loop through each key and add it to the map with a value of true
    string comma = ",";
    size_t loc = 0;
    string tokenB;
    string keysB(bufferB);
    while ((loc = keysB.find(comma)) != string::npos)
    {
        tokenB = keysB.substr(0, loc);
        serverBMap[tokenB] = true;
        keysB.erase(0, loc + comma.length());
    }
    cout << "Main Server received the username list from server B using UDP over port " << SERVERM_UDP << "."<<endl;
    cout << endl;
    cout << endl;

    // Repurposed from Beej’s socket programming tutorial
    // Accept connection from client using child socket
    socklen_t clientAddrSize = sizeof(destClient_addr);
    childSocketFD = ::accept(serverM_clientFD, (struct sockaddr *)&destClient_addr, &clientAddrSize);
    if (childSocketFD == FAIL)
    {
        perror("[ERROR] Server M failed to accept connection with Client.");
        exit(1);
    }

//while loop to continuously receive requests from client
while (true)
    {
        //PHASE 2
        string usernames = "";
        // cout << "DEBUG:: accepting connections " << endl;

        // To receive input usernames from client over TCP
        char buffer_client[MAX_USERNAME_LENGTH * 10]; // Max 10 usernames, each with a length of MAX_USERNAME_LENGTH
        int bytes_received = recv(childSocketFD, buffer_client, sizeof(buffer_client), 0);
        if (bytes_received == -1)
        {
            cerr << "Error receiving data from client" << endl;
            return 1;
        }
        buffer_client[bytes_received] = '\0';
        cout << "Main Server received the request from client using TCP over port " << SERVER_TCP_PORT << "." << endl;

        //Parsing usernames received from client
        vector<string> usernamesFromClient;
        char *client_recv_name = strtok(buffer_client, " ");
        while (client_recv_name != NULL)
        {
            usernamesFromClient.push_back(client_recv_name);
            client_recv_name = strtok(NULL, " ");
        }

        // Iterating over usernames and adding to sublists depending on the backend server that they belong to
        vector<string> sublistA, sublistB, sublistC;
        for (const auto &username_entered : usernamesFromClient)
        {
            if (serverAMap.find(username_entered) != serverAMap.end())
            {
                sublistA.push_back(username_entered);
            }
            else if (serverBMap.find(username_entered) != serverBMap.end())
            {
                sublistB.push_back(username_entered);
            }
            else
            {
                sublistC.push_back(username_entered);
            }
        }


        //Initializing variables to pass into CommonTimeAvailability algoritm.
        vector<pair<int, int>> intervals1;
        vector<pair<int, int>> intervals2;

        //Check if sublistA is empty and if not send those usernames to Server A for further processing.
        if (sublistA.empty())
        {
            //do nothing
        }
        else
        {
            if (!sublistA.empty())
            {
            cout << "Found ";
            // Loop over the elements of the sublist to print.
            for (std::size_t i = 0; i < sublistA.size(); i++)
            {
                cout << sublistA[i];
                // Print a comma if it's not the last element
                if (i < sublistA.size() - 1)
                {
                    cout << ", ";
                }
            }
            cout << " located at Server A. Send to Server A." << endl;
            }

            Phase2_sendServer_A_B(sublistA, sendaddrA);


            // PHASE 3
            // Server A
            char buffer_phase3_A[BUFFER_SIZE];

            // Receive the intersection result from Server A
            int num_bytes_received_A = recvfrom(sockfd_UDP, buffer_phase3_A, BUFFER_SIZE, 0, (struct sockaddr *)&sendaddrA, &sendaddrA_len);
            if (num_bytes_received_A == -1)
            {
                //cout << "DEBUG:: if (num_bytes_received_A == -1) HIT" << endl;
                perror("Error in receiving data");
                exit(EXIT_FAILURE);
            }
            buffer_phase3_A[num_bytes_received_A] = '\0';
            usernames = "user exists";
            cout<< "Main Server received from server A the intersection result using UDP over port " << SERVERM_UDP<< ":" <<endl;
            cout<< buffer_phase3_A << endl;
            // Parsing buffer data from server A  using the ParseIntervals function
            intervals1 = ParseIntervals(buffer_phase3_A);
       }

        //Check if sublistB is empty and if not send those usernames to Server B for further processing.
        if (sublistB.empty())
        {
            // do nothing
        }
        else
        {
             // Print sublists
            if (!sublistB.empty())
            {
            // Loop over the elements of the sublist to print
            for (std::size_t i = 0; i < sublistB.size(); i++)
            {
                cout << sublistB[i];

                // Print a comma if it's not the last element
                if (i < sublistB.size() - 1)
                {
                    cout << ", ";
                }
            }
            cout << " located at Server B. Send to Server B." << endl;
            }

            Phase2_sendServer_A_B(sublistB, sendaddrB);

           // PHASE 3
           // Server B
            char buffer_phase3_B[BUFFER_SIZE];
            // Receive the intersection result from Server B
            int num_bytes_received_B = recvfrom(sockfd_UDP, buffer_phase3_B, BUFFER_SIZE, 0, (struct sockaddr *)&sendaddrB, &sendaddrB_len);
            if (num_bytes_received_B == -1)
            {
                perror("Error in receiving data");
                exit(EXIT_FAILURE);
            }
            buffer_phase3_B[num_bytes_received_B] = '\0';
            usernames = "user exists";
            cout << "Main Server received from server B the intersection result using UDP over port " << SERVERM_UDP <<": " << endl;
            cout<<buffer_phase3_B <<"."<< endl;
            intervals2 = ParseIntervals(buffer_phase3_B);
        }

        //For usernames that are not present in both backend servers
        if (sublistC.size() > 0)
        {
            string user_does_not_exist = "";
            for (auto iter = sublistC.begin(); iter != sublistC.end(); ++iter)
            {
                if (iter != sublistC.begin())
                {
                    cout << ", ";
                    user_does_not_exist += ", ";
                }
                cout << *iter;
                user_does_not_exist += *iter;
            }
            cout << " do not exist. Send a reply to the client." << endl;
            //cout << "DEBUG:: sending from sublistC " << endl;
            usernames = user_does_not_exist + " ";
        }

        //PHASE 4
        //Running CommonTimeAvailability algorithm on resulting vectors
        vector<pair<int, int>> common_intervals;
        if (intervals1.empty() && !intervals2.empty())
        {
            common_intervals = intervals2;
        }
        else if (!intervals1.empty() && intervals2.empty())
        {
            common_intervals = intervals1;
        }
        else
        {
            common_intervals = CommonTimeAvailability(intervals1, intervals2);
        }

        if (!common_intervals.empty())
        {
            cout << "Found the intersection between the results from server A and B: " << endl;
                for (const auto &interval : common_intervals)
                {
                    cout << "[" << interval.first << "," << interval.second << "] ";
                }
                cout << endl;
        }

        // Formatting the final interval to the client
        stringstream final_interval;
        if (common_intervals.empty())
        {
            final_interval << "[] \n";
        }
        else
        {
            for (const auto &interval : common_intervals)
            {
                final_interval << "[" << interval.first << "," << interval.second << "] ";
            }
            final_interval << "\n";
        }

        stringstream final_username_list;
        for (const auto &username : sublistA)
        {
            final_username_list << username << " ";
        }

        for (const auto &username : sublistB)
        {
            final_username_list << username << " ";
        }

        string fresult;
        // adding not in databse usernames
        if (usernames.empty() || usernames == "user exists")
        {
            fresult = "[]";
        }
        else
        {
            std::stringstream ss;
            ss << '[' << usernames << ']';
            fresult += ss.str();
        }

        //Sending final intersection result to client
        string send_intersection = final_interval.str() + fresult + '\n';
        if (send(childSocketFD, send_intersection.c_str(), send_intersection.length(), 0) < 0)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }

        if (final_username_list.str().empty())
        {
            final_username_list << "[]";
        }

        if (send(childSocketFD, final_username_list.str().c_str(), final_username_list.str().size(), 0) < 0)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }

        cout << "Main Server sent the result to the client." << endl;
        cout << endl;
        cout << endl;
    }
}
