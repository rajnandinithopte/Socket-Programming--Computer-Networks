/*
Author: Rajnandini Thopte


client.cpp

This code is for the client that takes the input of usernames from the user between whom the meeting needs to be scheduled.
It sneds these names to the Main server for processing and prints the time intersection result it receives as well as if usernames
are not present in database to console.
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

#define LOCALHOST "127.0.0.1"
#define SERVER_PORT 24463
#define MAX_USERNAME_LENGTH 20
#define MAXBUFFERSIZE 10000
#define FAIL -1
#define _OE_SOCKETS

using namespace std;

int main()
{
    // Repurposed from Beej’s socket programming tutorial
    // Get dynamic port number for client
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Repurposed from Beej’s socket programming tutorial
    // Resolve server address
    struct hostent *server = gethostbyname(LOCALHOST);
    if (server == nullptr)
    {
        std::cerr << "Error resolving server address" << std::endl;
        return 1;
    }


    // Create server address structure
    struct sockaddr_in server_address;
    std::memset(&server_address, 0, sizeof server_address);
    server_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, LOCALHOST, &server_address.sin_addr) <= 0)
    {
        std::cerr << "Invalid IP address format" << std::endl;
        close(client_fd);
        return 1;
    }
    server_address.sin_port = htons(SERVER_PORT);

    // Connect to server
    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    //Establish client dynamic socket
    sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    socklen_t len = sizeof(my_addr);
    getsockname(client_fd, (struct sockaddr *)&my_addr, &len);
    unsigned int portNum = ntohs(my_addr.sin_port);

    printf("Client is up and running. \n");

    //While loop to take continuous inputs from user
    while (true)
    {
        // Prompt for user input
        char prompt[] = "Please enter the usernames to check schedule availability: ";
        std::cout << prompt << std::flush;

        // Get input from user
        char input[MAX_USERNAME_LENGTH * 10]; // Max 10 usernames, each with a length of MAX_USERNAME_LENGTH
        std::cin.getline(input, sizeof input);


        //std::cout << "DEBUG:: about to send data " << endl;

        // Send user input to main server
        if (send(client_fd, input, strlen(input), 0) == -1)
        {
            std::cerr << "Error sending data to server" << std::endl;
            return 1;
        }
        std::cout << "Client finished sending the usernames to Main Server." << std::endl;

        //Receive names not in database from server M
        char not_in_database[MAXBUFFERSIZE];
        std::memset(not_in_database, 0, sizeof(not_in_database));

        //To print user not present in database
        if (std::string(not_in_database) != "" && std::string(not_in_database) != "user exists")
        {
            std::cout << "Client received the reply from Main Server using TCP over port "
                      << portNum << std::endl;
            std::cout << not_in_database << " usernames do not exist." << std::endl;
        }

        else
        {

            char buffer_interval[MAXBUFFERSIZE];
            std::memset(buffer_interval, 0, sizeof(buffer_interval));

            // Receive up to the newline character to get buffer_interval
            int bytes_received = 0;
            char *p = buffer_interval;

            //std::cout << "DEBUG:: START while loop in client 1 byte at a time " << endl;
            while (bytes_received < MAXBUFFERSIZE && recv(client_fd, p, 1, 0) == 1)
            {
                if (*p == '\n')
                {
                    // Found the newline character, stop receiving
                    break;
                }
                p++;
                bytes_received++;
            }

            //std::cout << "DEBUG:: END while loop in client 1 byte at a time " << endl;

            // Null-terminate the received message
            buffer_interval[bytes_received] = '\0';

            // Convert the received data into a string
            std::string data_received(buffer_interval, bytes_received);

            char buffer_missing_names[MAXBUFFERSIZE];
            std::memset(buffer_missing_names, 0, sizeof(buffer_missing_names));

            // Receive up to the newline character to get buffer_missing_names
            bytes_received = 0;
            char *ptr = buffer_missing_names;

            while (bytes_received < MAXBUFFERSIZE && recv(client_fd, ptr, 1, 0) == 1)
            {
                if (*ptr == '\n')
                {
                    // Found the newline character, stop receiving
                    break;
                }
                ptr++;
                bytes_received++;
            }

            //Receiving usernames that do not exist from Main Server
            std::string missing_names_db(buffer_missing_names, bytes_received);

            if (missing_names_db != "[]" && missing_names_db != "user exists")
            {
                missing_names_db = missing_names_db.substr(1, missing_names_db.length() - 2);
                std::cout << "Client received the reply from Main Server using TCP over port " << portNum << ": " << endl << missing_names_db << "do not exist." << endl;
            }

            //User present and display intersection
            char buffer_names[MAXBUFFERSIZE];
            std::memset(buffer_names, 0, sizeof(buffer_names));

            //std::cout << "DEBUG:: START names_received " << endl;
            int names_received = recv(client_fd, buffer_names, sizeof(buffer_names), 0);
            if (names_received < 0)
            {
                perror("Error receiving data from server");
                exit(1);
            }

            // Null-terminate the received message
            buffer_names[names_received] = '\0';
            //std::cout << "DEBUG:: END names_received " << endl;

            // Convert the received data into a string
            std::string final_names(buffer_names, names_received);
            // Create a new string variable to store the modified names
            std::string modified_names;

            // Add a comma after every name except the last one, and append to the new string variable
            for (std::size_t i = 0; i < final_names.size(); i++)
            {
                if (final_names[i] == ' ')
                {
                    if (i < final_names.size() - 1)
                    {
                        modified_names += ", ";
                    }
                }
                else
                {
                    modified_names += final_names[i];
                }
            }

            // Format and print the received data
           if (data_received != "[]" && modified_names != "[]")
            {
            std::cout << "Client received the reply from Main Server using TCP over port " << portNum <<": " << endl << "Time intervals " << data_received << "works for " << modified_names << "." << std::endl;
            }
        }

        // Sleep for some time before sending another message
        sleep(1);
        std::cout << "-----Start a new request-----" << std::endl;
    }
    return 0;
}
