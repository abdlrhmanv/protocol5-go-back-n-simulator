# Protocol 5 Simulation (Tanenbaum - Chapter III)

Professional project layout for a Protocol 5 (Go-Back-N style) simulation.

## Project Structure

- `include/`: public headers and module contracts
- `src/`: implementation files (`main`, sender, receiver, channel)
- `Makefile`: standard build/run/clean workflow

## Build

```bash
make
```

## Run

```bash
./protocol5_sim [frame_loss_percent] [ack_loss_percent] [total_frames] [max_time_ms]
```

Defaults if omitted:

- frame loss: `0`
- ACK loss: `0`
- total frames: `12`
- max simulation time: `8000`

## Example Scenarios (for assignment snapshots)

```bash
./protocol5_sim 0 0 12 8000
./protocol5_sim 20 0 12 12000
./protocol5_sim 20 20 12 15000
```

## Team Submission Notes

- Submit one final file/package per group as required by your instructor.
- Include all member names in the required location (inside file/PDF/comment).
- Include snapshots of inputs and matching outputs from terminal runs.
# protocol5-go-back-n-simulator
