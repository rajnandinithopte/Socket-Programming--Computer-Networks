# Socket-Programming-Computer-Networks

## Developed a **meeting scheduling system** using **UNIX socket programming** with TCP and UDP communication.

---

## 🔹 Tech Stack

**Programming Language:** C++  
**Networking:** UNIX Sockets, TCP, UDP  
**Development Environment:** Ubuntu 22.04 

**Libraries & Tools:**
- **Beej’s Guide to Network Programming** - Socket programming reference
- **POSIX Sockets** - Implementing client-server communication
- **Makefile** - For compiling and running executables

---

## 🔹 System Overview

![System Architecture](architecture.png)

This project implements a **meeting scheduler** that:
- Enables users to **schedule a meeting with a group of people**.
- Uses **a client, main server, and two backend servers**.
- Communicates between **client & main server using TCP** and **main server & backend servers using UDP**.
- Finds **common available meeting slots** for multiple users.

The system consists of **four main components**:
1. **Client (client.cpp)** - Takes user input (names) and requests available time slots.
2. **Main Server (serverM.cpp)** - Manages user requests and forwards them to backend servers.
3. **Backend Server A (serverA.cpp)** - Stores availability data for a subset of users.
4. **Backend Server B (serverB.cpp)** - Stores availability data for another subset of users.

---

## 🔹 Workflow & Communication

1. **Boot-up Phase**
   - The **Main Server (ServerM)** starts and listens for incoming connections.
   - **Backend Servers (ServerA & ServerB)** load user availability data from input files (`a.txt`, `b.txt`).
   - **Backend Servers** send the list of stored users to **Main Server** via **UDP**.

2. **Request & Forwarding Phase**
   - The **Client** sends a list of participants to the **Main Server** via **TCP**.
   - The **Main Server** determines which **Backend Server** stores the required users' availability.
   - The **Main Server** forwards the request to the correct **Backend Server** via **UDP**.

3. **Scheduling Phase**
   - Each **Backend Server** finds the common available time slots for the requested users.
   - The **Backend Server** returns the result to the **Main Server**.

4. **Reply & Final Intersection Phase**
   - The **Main Server** aggregates availability from both **Backend Servers**.
   - The final list of available meeting times is sent back to the **Client** via **TCP**.

---

## 🔹 Code Files & Their Purpose

### 1. **serverM.cpp** (Main Server)
- Handles **client requests** and forwards them to the correct backend servers.
- Creates a **TCP socket** to communicate with the **Client**.
- Creates a **UDP socket** to communicate with **Backend Servers**.
- Determines if the user belongs to `serverA` or `serverB` based on input files.
- **Aggregates** the final meeting time slots from both backend servers.

### 2. **serverA.cpp** (Backend Server A)
- Creates a **UDP socket** to communicate with the **Main Server**.
- Reads **user availability data** from `a.txt` and stores it in a **map data structure**.
- Finds **common available time slots** for requested users.
- Sends the result back to **Main Server**.

### 3. **serverB.cpp** (Backend Server B)
- Creates a **UDP socket** to communicate with the **Main Server**.
- Reads **user availability data** from `b.txt` and stores it in a **map data structure**.
- Finds **common available time slots** for requested users.
- Sends the result back to **Main Server**.

### 4. **client.cpp** (Client Program)
- Creates a **TCP socket** to communicate with **Main Server**.
- Accepts **usernames as input** (max 10 usernames, max 20 characters each).
- Sends **user requests** to the **Main Server**.
- Receives and **displays the available meeting slots**.

---

## 🔹 Constraints Considered While Developing the Algorithm

### **🔸 Efficiency**
- **User list processing (hashmap lookup):** `O(U)`
- **Sorting user intervals before merging:** `O(N log N)`
- **Merging time slots for intersection:** `O(N)`
- **Sending & receiving data from backend servers:** `O(K)`
- **Overall Time Complexity:** `O(U log U + N log N)`

**Final Complexity:**  
`O(U log U + N log N)`,  
where **U = number of users in request** and **N = number of time intervals processed**.

- **Pairwise intersection checking** ensures that only **valid overlapping intervals** are considered.
- **Memory Efficiency:**  
  - Uses **maps or hash tables** to store and retrieve user availability efficiently.
  - Can handle **large input sizes (up to 200 lines per file) without excessive memory consumption**.

---

### **🔸 Input Validation & Formatting**
- **Ensures Correct Time Intervals:**
  - **Start time < End time** for every time slot.
  - Each interval must be **strictly increasing** (i.e., `t[i]_end < t[i+1]_start`).
- **Handles Formatting Issues:**
  - **Removes unwanted spaces** before processing.
  - Ensures that **usernames contain only lowercase letters** (as required).
  - **Rejects invalid or malformed time intervals** (e.g., `[[1,1]]` or `[[10,5]]` are invalid).
- **Handles Cases Where No Intersection Exists:**
  - If **no overlapping time slots** exist, the algorithm correctly returns **an empty list (`[]`)**.

---

### **🔸 Scalability & Performance**
- **Handles Large Datasets Efficiently:**
  - The algorithm is optimized for processing **large input files (up to 200 lines each)**.
  - Efficient use of **sorting and merging** prevents excessive computation time.
- **Designed to Process Multiple Users in One Query:**
  - Supports up to **10 usernames per request**.
  - Uses **iterative intersection** to process multiple users efficiently.

---

### **🔸 Robust Handling of Edge Cases**
- **Handles Single User Queries Efficiently:**
  - If a **single user** is requested, the backend directly **returns their full availability** without unnecessary computations.
- **Handles Very Short and Very Long Time Slots:**
  - Works with **small intervals** like `[[1,2]]` and **large intervals** like `[[0,100]]`.
- **Correctly Processes Scenarios Where Users Exist on Different Backend Servers:**
  - Ensures that the **Main Server** correctly assigns **each user to the correct backend server**.
  - Aggregates results from multiple backend servers correctly.

---

### **🔸 Consistency & Accuracy**
- **Results Are Always Sorted and Maintain Valid Overlaps:**
  - Ensures **final intersection lists remain sorted**.
  - Guarantees that **all overlapping intervals are preserved**.
- **Ensures All Time Slots Are Fully Processed:**
  - Even if a user has **multiple separate availability slots**, the algorithm **correctly computes intersections across all of them**.
- **Adheres to On-Screen Message Requirements:**
  - **Backend servers print** results after computing intersections.
  - **Main Server correctly forwards** results and prints appropriate messages.

---

# 🔹 Algorithm for Finding Common Meeting Slots

The **meeting scheduler algorithm** ensures that a meeting can be scheduled by finding **common available time slots** among multiple users stored across two backend servers.

---

## **🔸Step 1: Backend Servers Read and Store Data**
- **Backend Server A (ServerA)** reads `a.txt` and stores availability data for **a subset of users**.
- **Backend Server B (ServerB)** reads `b.txt` and stores availability data for **another subset of users**.
- Each **backend server** sends a list of usernames it manages to the **Main Server** via **UDP**.

### **Example Data in `a.txt` and `b.txt`**
```
a.txt:
alice;[[1,10],[11,12]]
bob;[[5,9],[11,15]]

b.txt:
amy;[[4,12]]
charlie;[[3,8],[9,15]]
```
At this stage, **ServerA manages Alice and Bob**, while **ServerB manages Amy and Charlie**.

---

## **🔸 Step 2: Client Sends a Meeting Request**
1. The **client** enters names of participants.
2. The **client program** sends the list to the **Main Server** via **TCP**.
3. The **Main Server** checks which **backend server** stores each user’s data.
4. It **splits the list** and **forwards requests via UDP** to the respective backend servers.

### **Example Client Request**
```
User Input: alice bob amy
```
The **Main Server** identifies:
- `alice` and `bob` → Stored in **ServerA**
- `amy` → Stored in **ServerB**

Thus, the **Main Server** sends:
- **To ServerA**: Request for **alice, bob** availability.
- **To ServerB**: Request for **amy** availability.

---

## **🔸 Step 3: Backend Servers Compute Individual Intersections**
Each backend server:
1. **Finds the time availability** for requested users.
2. **Computes the pairwise intersection** of time slots.
3. **Sends the result back to the Main Server**.

### **Case 1: One User**
- If only **one user** is in the request, the backend server directly sends **their availability**.

#### **Example**
```
User Input: amy
```
Amy’s availability from `b.txt`: `[[4,12]]`  
Backend Server B returns:  
```
[[4,12]]
```
---

### **Case 2: Two Users**
If two users exist, the backend server:
- **Fetches both users’ time slots**.
- **Finds overlapping intervals**.

#### **Example (Alice & Bob)**
```
Alice: [[1,10],[11,12]]
Bob: [[5,9],[11,15]]
```
**Algorithm computes intersections**:
- `[1,10]` and `[5,9]` → Intersection: **`[5,9]`**
- `[11,12]` and `[11,15]` → Intersection: **`[11,12]`**

#### **Backend Server A returns result**  
```
[[5,9],[11,12]]
```
---

### **Case 3: More than Two Users**
If there are **three or more** users, the algorithm:
1. **Finds the intersection between the first two users**.
2. **Uses the result to find an intersection with the next user**.
3. **Repeats this process until all users' availability is considered**.

#### **Example (Alice, Bob, and Amy)**
```
Alice: [[1,10],[11,12]]
Bob: [[5,9],[11,15]]
Amy: [[4,12]]
```
**Step 1: Find Alice & Bob’s intersection**
```
[[5,9],[11,12]]
```
**Step 2: Find intersection of result with Amy**
```
[[5,9],[11,12]] ∩ [[4,12]]
```
**Final Intersection Result**:  
```
[[5,9],[11,12]]
```
Backend Servers send results to the **Main Server**.

---

## **🔸 Step 4: Main Server Computes Final Intersection**
1. **Main Server receives results** from **ServerA and ServerB**.
2. **It runs another intersection algorithm** between the two sets.

#### **Example**
```
Server A result: [[6,7],[10,12],[14,15]]
Server B result: [[3,8],[9,15]]
```
 **Final Intersection Computed by Main Server**  
```
[[6,7],[10,12],[14,15]]
```
**Main Server sends this to the client.**

---

## **🔸 Step 5: Client Receives and Displays Final Meeting Slots**
Once the **client receives the final result**, it **displays the available meeting times**:

#### **Example Client Output**
```
Time intervals [[6,7],[10,12],[14,15]] work for alice, bob, amy.
```

 **If no common slots exist, it returns** `[]`.

---

# 🔹Port Assignments

| Process  | Protocol | Port Number|
|----------|----------|------------|
| Server A | UDP      | **21+XXX** |
| Server B | UDP      | **22+XXX** |
| Server M | UDP      | **24+XXX** |
| Server M | UDP      | **23+XXX** |
| Client   | TCP      | **Dynamic**|

Each process communicates using the specified **ports** to ensure correct message routing.


# 🔹How to Run the Meeting Scheduling System

## 1. Compile the Code
Use the `Makefile` to compile all server and client programs:
```sh
make all
```

This will generate the following executable files:
- **serverM** (Main Server)
- **serverA** (Backend Server A)
- **serverB** (Backend Server B)
- **client** (Client Program)

---

## 2.  Start the Servers and Client

### Step 1: Start the Main Server
Open a new terminal window and run:
```sh
./serverM
```
**Expected Output:**
```
Main Server is up and running.
```

### Step 2: Start Backend Server A
In another terminal window, run:
```sh
./serverA
```
**Expected Output:**
```
Server A is up and running using UDP on port <port number>.
Server A finished sending a list of usernames to Main Server.
```

### Step 3: Start Backend Server B
In a third terminal window, run:
```sh
./serverB
```
**Expected Output:**
```
Server B is up and running using UDP on port <port number>.
Server B finished sending a list of usernames to Main Server.
```

### Step 4: Start the Client
In a fourth terminal window, run:
```sh
./client
```
**Expected Output:**
```
Client is up and running.
Please enter the usernames to check schedule availability:
```

---

## 3. Enter User Input in the Client
Once the client is running, enter a list of usernames:
```
Please enter the usernames to check schedule availability:
alice bob charlie
```

The client will process the request and return available time slots.

---

## 4. Communication Flow & Expected Messages

### 1. Client to Main Server
After sending the request:
```
Client finished sending the usernames to Main Server.
```

If some usernames do not exist:
```
Client received the reply from Main Server using TCP over port <port number>:
<username1, username2, …> do not exist.
```

If usernames exist:
```
Main Server received the request from client using TCP over port <port number>.
Found <username1, username2, …> located at Server <A or B>. Send to Server <A or B>.
```

### 2. Main Server to Backend Servers
```
Main Server received the username list from server A using UDP over port <port number>.
Main Server received the username list from server B using UDP over port <port number>.
```

Each backend server will process its data and return the intersection:
```
Server <A or B> received the usernames from Main Server using UDP over port <port number>.
Found the intersection result: <[[t1_start, t1_end], [t2_start, t2_end], … ]> for <username1, username2, …>.
Server <A or B> finished sending the response to Main Server.
```

### 3. Main Server Processing the Final Result
```
Main Server received from server A the intersection result using UDP over port <port number>:
<[[t1_start, t1_end], [t2_start, t2_end], … ]>.
Main Server received from server B the intersection result using UDP over port <port number>:
<[[t1_start, t1_end], [t2_start, t2_end], … ]>.
Found the intersection between the results from server A and B:
<[[t1_start, t1_end], [t2_start, t2_end], … ]>.
Main Server sent the result to the client.
```

### 4. Client Receives the Final Meeting Time

```
Client received the reply from Main Server using TCP over port <port number>:
Time intervals <[[t1_start, t1_end], [t2_start, t2_end], … ]> works for <username1, username2, …>.
```

---

## 5. Stop the Servers
Once you're done testing, stop all servers and the client by pressing:
```sh
CTRL + C
```
This will terminate the processes.


## 🔹 References
This project references concepts from:
1. **Beej's Guide to Network Programming**: [Beej's Guide](http://www.beej.us/guide/bgnet/)
   - Chapter 3: `structs`
   - Chapter 4: `socket(), bind(), connect(), listen(), accept(), send()/recv(), sendto()/recvfrom()`
   - Chapter 9.12: `htons()`
   - Chapter 9.24: `struct sockaddr_in`

