<p align="center">
  <img src="./img/en_logo.png" alt="Ain Shams University Faculty of Engineering" width="560"/>
</p>

<h1 align="center">Protocol 5 Go-Back-N Simulator</h1>

<p align="center">
  <em>Computer Networks Project (Chapter III - Tanenbaum)<br/>
  Ain Shams University - Faculty of Engineering</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white" alt="C++17">
  <img src="https://img.shields.io/badge/Compiler-g%2B%2B-blue" alt="g++">
  <img src="https://img.shields.io/badge/Protocol-Go--Back--N-success" alt="Go-Back-N">
  <img src="https://img.shields.io/badge/Platform-Linux-FCC624?logo=linux&logoColor=black" alt="Linux">
</p>

---

## Team Members

| Student Name | ID |
| :-- | :--: |
| Sama Gamal Ahmed Salama | 2300371 |
| Menna Osama | 2300515 |
| Malak Mamdoh | 2300428 |
| Ahmed Moataz Hefny | 2301040 |
| Abdlrhman Hisham Ismail | 2300343 |

---

## Project Overview

This repository contains a **Protocol 5 simulation** in C++ using **Go-Back-N style sliding window** behavior.  
The simulation models sender/receiver interaction over a lossy channel with propagation delay and cumulative ACK handling.

---

## Repository Structure

```text
protocol5-go-back-n-simulator/
├── include/
│   ├── Header.h
│   └── Channel.h
├── src/
│   ├── main.cpp
│   ├── Sender.cpp
│   ├── Receiver.cpp
│   └── Channel.cpp
├── img/
│   └── en_logo.png
└── README.md
```

---

## Build & Run

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude src/main.cpp src/Sender.cpp src/Receiver.cpp src/Channel.cpp -o protocol5_sim
./protocol5_sim [frame_loss_percent] [ack_loss_percent] [total_frames] [max_time_ms]
```

Default values (if omitted):

- frame loss = `0`
- ACK loss = `0`
- total frames = `12`
- max simulation time = `8000`

---

## Example Scenarios

```bash
./protocol5_sim 0 0 12 8000
./protocol5_sim 20 0 12 12000
./protocol5_sim 20 20 12 15000
```

---

## Submission Notes

- Include snapshots of command inputs and corresponding outputs.
- Submit group names as required by the course instructions.
- Keep this README with the final submitted project package/file.
