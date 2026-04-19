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

This repository contains a **Protocol 5 (Go-Back-N) simulation** in C++.
It mirrors the implementation given in Tanenbaum & Wetherall, *Computer
Networks*, 5e (Fig. 3.19 / Fig. 3.20):

- 3-bit sequence-number space (`MAX_SEQ = 7`, window size = `MAX_SEQ`).
- Single unified `frame` struct with `kind` / `seq` / `ack` / `info`
  (ACKs are sent on dedicated `FK_ACK` frames since the simulation has no
  reverse data traffic; with bidirectional traffic ACKs would simply
  piggyback on outbound `FK_DATA` frames).
- Sender uses circular buffer indexing (`seq % NR_BUFS`) and the textbook
  `between(a, b, c)` window test.
- **Per-frame timer list** (`start_timer(seq)` / `stop_timer(seq)`) modeled
  after Fig. 3.20.
- Channel models loss, **checksum corruption** (`cksum_err` event),
  per-frame transmission time, and one-way propagation delay.
- Reproducible runs via a CLI seed argument.

---

## Repository Structure

```text
protocol5-go-back-n-simulator/
├── include/
│   ├── Header.h        # frame model, protocol constants, sender/receiver API
│   ├── Channel.h       # bidirectional lossy/corrupting channel API
│   └── Timer.h         # multi-timer API (Fig. 3.20)
├── src/
│   ├── main.cpp        # simulation driver + CLI
│   ├── Sender.cpp      # protocol5() sender side
│   ├── Receiver.cpp    # protocol5() receiver side
│   ├── Channel.cpp     # channel simulation
│   └── Timer.cpp       # multi-timer implementation
├── img/
│   └── en_logo.png
└── README.md
```

---

## Build & Run

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude \
    src/main.cpp src/Sender.cpp src/Receiver.cpp src/Channel.cpp src/Timer.cpp \
    -o protocol5_sim

./protocol5_sim [frame_loss%] [ack_loss%] [total_frames] [max_time_ms] [corrupt%] [seed]
```

| Argument        | Default | Meaning |
| :-------------- | :-----: | :------ |
| `frame_loss%`   | `0`     | Drop probability for A→B frames (%) |
| `ack_loss%`     | `0`     | Drop probability for B→A frames (%) |
| `total_frames`  | `12`    | Number of application packets to deliver |
| `max_time_ms`   | `8000`  | Simulation horizon in ms |
| `corrupt%`      | `0`     | Probability the channel corrupts a delivered frame (%) — triggers the `cksum_err` event |
| `seed`          | `0`     | PRNG seed; `0` seeds from wall-clock time |

---

## Example Scenarios

```bash
# Happy path
./protocol5_sim 0 0 12 8000 0 1

# Loss only
./protocol5_sim 30 0 12 30000 0 42

# Loss + corruption (cksum_err events)
./protocol5_sim 20 20 12 30000 20 7

# Stress: many frames with deep wraparound
./protocol5_sim 0 0 200 60000 0 1
```

---

## Mapping to Tanenbaum's Pseudocode

| Textbook concept                   | Where it lives in this code                        |
| :---------------------------------- | :------------------------------------------------- |
| `frame` struct (kind, seq, ack)    | `frame` in `include/Header.h`                      |
| `between(a, b, c)`                 | `between()` in `src/Sender.cpp`                    |
| `send_data()`                      | `send_data()` (anonymous ns) in `src/Sender.cpp`   |
| `network_layer_ready` event        | `sender_from_network_layer()`                      |
| `frame_arrival` event              | `sender_on_frame_arrival()` / `receiver_on_frame_arrival()` |
| `cksum_err` event                  | `corrupt == true` branch in both handlers          |
| `timeout` event                    | `sender_on_timeout()` (driven by `pop_expired_timer`) |
| `start_timer` / `stop_timer`       | `src/Timer.cpp` (multi-timer per Fig. 3.20)        |
| Cumulative ACK loop                | `while (between(ack_expected, f.ack, next_frame_to_send))` in sender |

---

## Submission Notes

- Include snapshots of command inputs and corresponding outputs.
- Submit group names as required by the course instructions.
- Keep this README with the final submitted project package/file.
