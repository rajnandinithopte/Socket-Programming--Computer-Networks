/*
Author: Rajnandini Thopte

serverB.cpp

This code is for backend server A. It has access to all the usernames and their availabilities that are present in a.txt.
It stores and parses the information, sends usernames to client and also finds the common time availability for users present in its database
and sends it to the main server for further processing.
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
#include <algorithm>

#define SERVER_B 22463
#define SERVER_M 23463
#define BUFFER_SIZE 10000
#define LOCALHOST "127.0.0.1"
#define FAIL -1

using namespace std;

int serverB_sockfd;
struct sockaddr_in my_addr;
struct sockaddr_in serverM_addr;

vector<string> map_checklist;
// Define map as a global variable
map<string, vector<pair<int, int>>> databaseB_map;

// Repurposed from Beej’s socket programming tutorial
// Creates the UDP socket for ServerB
void create_serverB_socket()
{
    serverB_sockfd = socket(AF_INET, SOCK_DGRAM, 0); // creates a UDP socket
    if (serverB_sockfd == FAIL)
    {
        perror("[ERROR] Server B cannot open socket.");
        exit(1);
    }
}

// Repurposed from Beej’s socket programming tutorial
// initializeConnection() sets the struct sockaddr_in for serverB with appropriate parameters and its' assigned UDP port number.
void initializeConnectionB()
{
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    my_addr.sin_port = htons(SERVER_B);
}

// Repurposed from Beej’s socket programming tutorial
void initializeConnectionM()
{
    memset(&serverM_addr, 0, sizeof(serverM_addr));
    serverM_addr.sin_family = AF_INET;
    serverM_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverM_addr.sin_port = htons(SERVER_M);
}

// Repurposed from Beej’s socket programming tutorial
void bindSocket()
{
    // Bind the socket to the receive address
    if (::bind(serverB_sockfd, (sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        perror("[ERROR] Server B failed to bind UDP socket.");
        exit(1);
    }
    cout << "Server B is up and running using UDP on port " << SERVER_B <<"."<< endl;
}

// Custom function to convert a string to an integer
int strToInt(const string &str)
{
    int res = 0;
    for (char c : str)
    {
        res = res * 10 + (c - '0');
    }
    return res;
}

//This function reads input file b.txt and parses it into a map data structure with usernames as keys and time availabilities as values.
vector<string> readInput()
{
    vector<string> usernames;
    // Open the input file
    ifstream input_file("b.txt");

    // Check if the file is opened successfully
    if (!input_file)
    {
        cerr << "Error: Could not open the input file" << endl;
        return usernames;
    }

    // Read the file line by line
    string line;
    while (getline(input_file, line))
    {
        if (line.length() == 0)
        {
            continue;
        }
        // Split the line by semicolon
        size_t pos = line.find(';');
        string name = line.substr(0, pos);
        string intervals = line.substr(pos + 2);
        name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
        usernames.push_back(name);

        // Split the ranges string by comma and brackets
        vector<pair<int, int>> ranges;
        pos = 0;
        while ((pos = intervals.find('[', pos)) != string::npos)
        {
            size_t end_pos = intervals.find(']', pos);
            string range_str = intervals.substr(pos + 1, end_pos - pos - 1);
            pos = end_pos + 1;

            // Split the range string by comma Eg- finding the comma between 5 and 6 ie 5, 6
            size_t comma_pos = range_str.find(',');
            // We use strToInt because currently we have a string and we want to store values as a integer
            int start = strToInt(range_str.substr(0, comma_pos));
            int end = strToInt(range_str.substr(comma_pos + 1));
            ranges.push_back(make_pair(start, end));
        }

        // Add the data to the map
        databaseB_map[name] = ranges;
    }
    // Close the input file
    input_file.close();
    return usernames;
}

//This is the algorithm that calculates the common time availability among multiple users.
vector<pair<int, int>> CommonTimeAvailability(vector<pair<int, int>> intervals1, vector<pair<int, int>> intervals2)
{
    vector<pair<int, int>> commonIntervals;
    int i = 0, j = 0;
    while (i < intervals1.size() && j < intervals2.size())
    {
        // check if the intervals overlap
        if (intervals1[i].second < intervals2[j].first)
        {
            i++;
        }
        else if (intervals2[j].second < intervals1[i].first)
        {
            j++;
        }
        else // there is an overlap or adjacent intervals
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

//Parsing the data received from Main server in Phase 2
string split_buffer(string buffer_phase2)
{
    // Split the buffer by spaces
    stringstream ss(buffer_phase2);
    string name_check;
    while (getline(ss, name_check, ' '))
    {
        map_checklist.push_back(name_check);
    }
    return name_check;
}

int main()
{
    // Create UDP socket for server B
    create_serverB_socket();
    // Create sockaddr_in struct
    initializeConnectionB();
    initializeConnectionM();
    bindSocket();

    // Reading input file
    vector<string> usernames = readInput();
    string data;
    for (string username : usernames)
    {
        data += username + ',';
    }

    //Sending usernames of b.txt to server M
    socklen_t addr_len = sizeof(serverM_addr);
    if (sendto(serverB_sockfd, (void *)data.c_str(), data.length(), 0, (struct sockaddr *)&serverM_addr, addr_len) == FAIL)
    {
        perror("Server B failed to send usernames to Server M");
        exit(1);
    }

    cout << "Server B finished sending a list of usernames to Main Server." << endl;
    cout << endl;
    cout << endl;

    //while loop for continuous requests
    while (true)
    {
        map_checklist.clear();

        char buffer_phase2[BUFFER_SIZE];
        socklen_t addr_len = sizeof(serverM_addr);

        //Receiving usernames from Main server for which we need to find common time intervals.
        int bytes_received = recvfrom(serverB_sockfd, buffer_phase2, BUFFER_SIZE, 0, (struct sockaddr *)&serverM_addr, &addr_len);
        if (bytes_received == FAIL)
        {
            perror("Server B did not receieve usernames from main server");
            exit(1);
        }

        buffer_phase2[bytes_received] = '\0';
        std::cout << "Server B received the usernames from Main Server using UDP over port " << SERVER_B << "." << std::endl;

        //Parsing the data received from Main server
        split_buffer(buffer_phase2);

        //Finding the time availabilities for usernames received from main server from map
        vector<pair<string, vector<pair<int, int>>>> selected_users;
        for (const auto &selected_name : map_checklist)
        {
            // Check if the username exists in the data map
            if (databaseB_map.find(selected_name) != databaseB_map.end())
            {
                // If it does, add it to the selected_users vector
                selected_users.push_back(make_pair(selected_name, databaseB_map[selected_name]));
            }
            else
            {
                // If it doesn't, print an error message
                cerr << "Error: No data found for user " << selected_name << endl;
            }
        }

        //Finding common time intersection for the usernames received from Main server.
        vector<pair<int, int>> time_intersection;
        if (selected_users.size() == 1)
        {
            time_intersection = selected_users[0].second;
        }
        else if (selected_users.size() > 1)
        {
            // If there are multiple users, find the common time intervals iteratively
            // Start with the time intervals of the first user
            time_intersection = selected_users[0].second;

            for (int i = 1; i < selected_users.size(); i++)
            {
                // Find the common intervals between the previously found common intervals
                // and the time intervals of the current user
                time_intersection = CommonTimeAvailability(time_intersection, selected_users[i].second);

            }
        }

          //Formatting the final time intersection
        string intersection_str = "";
        for (const auto &interval : time_intersection)
        {
            intersection_str += "[" + to_string(interval.first) + ", " + to_string(interval.second) + "] ";
        }

        if (time_intersection.empty())
        {
            intersection_str = "[]";
        }

        char intersection_arr[intersection_str.size() + 1];
        strcpy(intersection_arr, intersection_str.c_str());
        //Sending intersection result to Main server
        int size = intersection_str.size();
        char output_arr[size + 1];
        strcpy(output_arr, intersection_str.c_str());

        int num_bytes_sent = sendto(serverB_sockfd, output_arr, size, 0, (struct sockaddr *)&serverM_addr, sizeof(serverM_addr));
        if (num_bytes_sent == -1)
        {
            perror("Error in sending data");
            exit(EXIT_FAILURE);
        }

         //Formatting print statement
        if (time_intersection.empty())
        {
            string intersection_result = "Found intersection result ";
            intersection_result += "[] ";
            intersection_result += "for ";
            for (int i = 0; i < selected_users.size(); i++)
            {
                intersection_result += selected_users[i].first;
                // If this is not the last selected user, add a comma and space
                if (i < selected_users.size() - 1)
                {
                    intersection_result += ",";
                }
            }
        intersection_result += ".";
        cout << intersection_result << endl;
        }

        else
        {
        string intersection_result = "Found intersection result ";
        // Loop through each interval and add its string representation to the result string
        for (const auto &interval : time_intersection)
        {
        intersection_result += "[" + to_string(interval.first) + ", " + to_string(interval.second) + "] ";
        }

        // Add the names of the selected users to the result string
        intersection_result += "for ";
        for (int i = 0; i < selected_users.size(); i++)
        {
            intersection_result += selected_users[i].first;
            // If this is not the last selected user, add a comma and space
            if (i < selected_users.size() - 1)
            {
                intersection_result += ",";
            }
        }
        intersection_result += ".";
        cout << intersection_result << endl;
        }

        cout << "Server B finished sending the response to Main Server." << endl;
        cout << endl;
        cout << endl;
    }
    return 0;
}
