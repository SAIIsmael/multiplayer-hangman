# Multiplayer Hangman Game - Client/Server in C


![C](https://img.shields.io/badge/C-17-00599C)
![Multithreading](https://img.shields.io/badge/Multithreading-enabled-D32F2F)
![TCP/IP](https://img.shields.io/badge/TCP%2FIP-Protocol-D32F2F)
![Project Status](https://img.shields.io/badge/Status-Completed-brightgreen.svg)
![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)

This project implements a multiplayer Hangman game using a client-server architecture in C. The game allows multiple users to connect to a server using TCP sockets and play Hangman in real time with synchronized game state updates.

---

## Features

- **TCP Networking**: Efficient communication between client and server using custom `recvAll`, `sendAll`, and structured message sending.
- **Multithreading**: Client-side threading for real-time game state updates without blocking user input.
- **Dynamic GUI Rendering**: Terminal-based game interface with Hangman ASCII graphics and word progress visualization.
- **Game Synchronization**: Shared game state (`gamestate`) sent/received with support for error tracking and win/loss logic.
- **Robust Error Handling**: Handles disconnections and malformed inputs gracefully.

## Installation

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential libpthread-stubs0-dev
```

### MacOS (with homebrew)
```bash
brew install gcc
```

## Build 
1. Clone the repository (if you haven't already):
   ```bash
   git clone <your-repo-url>
   cd <your-project-directory>
   ```
2. Navigate to the build/ directory:
   ```bash
   cd build
   ```
3. Run make to compile the source code and create the executable:
   ```bash
   make
   ```
>This will compile the source code and create the client and server executables.

## Run the application
To start the server:
```bash
./server
``` 

to start the client:
```hash
./client
```

## File Structure
```
project-name/
├── src/          # C source files
│   ├── tcpClient.c  # Client-side TCP socket implementation and update thread
│   ├── tcpServer.c  # Server-side socket handling and main game loop
│   └── functions.c  # Shared utility functions like string management, initialization, and game logic
├── inc/          # Header files
│   └── functions.h  # Shared header file defining structures, constants, and function prototypes
├── build/        # Makefile and build target
│   └── makefile  # Build automation script
└── README.md     # Project documentation
```

## Sample gameplay
```
==========================================
|              HANGMAN                 |
==========================================

+------------- --------- -------------+	+--------------------+
|   Word : _ _ _ _                    |	|    error : 2/8     |
+------------- --------- -------------+	+--------------------+

Letter >
```

## Author
Ismael Sai Software engineer | Graphics & Simulation Enthusiast

## License
This project is released under the MIT License. Feel free to use it for learning or inspiration.

## Acknowledgments
Thanks to my university teammates for their collaboration during the research phase.
