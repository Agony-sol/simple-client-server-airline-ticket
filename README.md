# ✈️ Flight Ticketing System (C Client-Server)

A multithreaded Client-Server flight booking application written in C. This project simulates an airline ticketing system where multiple clients can concurrently search for flights, view available seats, and book tickets safely.

The communication is established using **UNIX Domain Sockets** (`AF_UNIX`), and the server handles concurrent client requests using **POSIX Threads** (`pthreads`).

## ✨ Features

* **Concurrent Connections:** The server spawns a new thread for every client that connects, allowing multiple users to interact with the system simultaneously.
* **Smart Flight Search:** Clients can input their origin and destination. The server searches its schedule and returns both **Direct Flights** and **Layover (1-stop) Flights**.
* **Thread-Safe Booking:** The server uses Mutex locks (`pthread_mutex_t`) for each individual flight. This ensures that if two clients try to book the exact same seat at the exact same time, no double-booking occurs (Race Condition prevention).
* **Record Keeping:** Successful bookings are logged into a local `flights.txt` file, recording the passenger's details, passport info, and itinerary.

## 🛠️ Technologies & Concepts

* **Language:** C
* **IPC (Inter-Process Communication):** UNIX Domain Sockets (`SOCK_STREAM`)
* **Concurrency:** POSIX Threads (`pthread_create`, `pthread_detach`)
* **Synchronization:** Mutexes (`pthread_mutex_lock`, `pthread_mutex_unlock`)

## 🚀 How to Build and Run

### 1. Compilation

You need to compile both the server and the client. The server requires the `-pthread` flag to link the POSIX threads library.

Open your terminal and run:

```bash
# Compile the server
gcc server.c -o server -pthread

# Compile the client
gcc client.c -o client
```

### 2. Execution

Since the project uses UNIX Domain Sockets, you need to provide a file path to act as the socket connection point. 

**Start the Server:**
Run the server first and pass the socket path as an argument.
```bash
./server /tmp/flight_socket.sock
```

**Start the Client(s):**
Open a new terminal window (you can open multiple to test concurrency) and run the client with the **exact same socket path**.
```bash
./client /tmp/flight_socket.sock
```

*(Note: The server will automatically unlink/delete the socket file when it terminates, but if it crashes, you may need to manually delete `/tmp/flight_socket.sock` before restarting it).*

## 📖 User Flow

1. The client connects to the server.
2. The user is prompted to enter their personal information (Name, Surname, Passport Country, Passport Number).
3. The user enters their **Origin** and **Destination** cities.
4. The server replies with a list of available flights (Direct and Layovers).
5. The user selects a flight route by its number.
6. The server replies with a list of available seats (1-10).
7. The user selects their seat(s).
8. The server locks the specific flight schedule, verifies seat availability, confirms the booking, and writes it to `flights.txt`.
