// =============================================================================
// sender.cpp
// -----------------------------------------------------------------------------
// Sender-side implementation for Protocol 5 (Go-Back-N behavior).
// =============================================================================

#include <iostream>   // Provides std::cout and std::endl for trace logging.
#include <vector>     // Provides std::vector for retransmission buffering.
#include "Header.h"   // Shared protocol data types and sender API declarations.
#include "Channel.h"  // Channel API used to transmit data frames.

using namespace std;  // Keeps source compact for educational readability.

// Tracks the sequence number of the oldest unacknowledged frame.
int base = 0;
// Tracks the sequence number to assign to the next outgoing frame.
int next_seq_num = 0;

// Stores already-sent frames by sequence number for potential retransmission.
vector<Frame> buffer(1000);

// Indicates whether the sender timeout timer is currently active.
bool timer_running = false;
// Stores simulation time at which the current timer started.
int timer_start_time = 0;

// Timeout threshold in milliseconds used for Go-Back-N retransmission.
const int TIMEOUT = 600;

// Attempts to send one new frame if the sender window has available space.
bool send_frame(int current_time)
{
    // Stop when send window [base, base + WINDOW_SIZE) is full.
    if (next_seq_num >= base + WINDOW_SIZE)
        return false;

    // Create a frame object for the next sequence number.
    Frame f;
    // Write the sequence number that uniquely identifies this frame.
    f.seq_num = next_seq_num;
    // Cache frame for retransmission in case of timeout.
    buffer[next_seq_num] = f;

    // Pass frame to channel simulator for delayed/lossy delivery.
    send_to_channel(f);
    // Log send event with simulation timestamp.
    cout << "t=" << current_time << ": Sent frame " << f.seq_num << endl;

    // Start timer only when first unacknowledged frame enters the channel.
    if (!timer_running)
    {
        // Mark timer as active.
        timer_running = true;
        // Record timer start time.
        timer_start_time = current_time;
    }

    // Advance next sequence number for future sends.
    next_seq_num++;
    // Report successful send to caller.
    return true;
}

// Handles cumulative ACKs delivered from receiver through the channel.
void receive_ack(int ack_num, int current_time)
{
    // Ignore duplicate/old ACKs that do not advance the sender window.
    if (ack_num < base)
        return;

    // Slide window base to first frame not covered by cumulative ACK.
    base = ack_num + 1;

    // Log ACK arrival for traceability.
    cout << "t=" << current_time
         << ": ACK " << ack_num << " received" << endl;

    // If everything sent so far is acknowledged, stop timer.
    if (base == next_seq_num)
    {
        // No outstanding frames remain.
        timer_running = false;
    }
    else
    {
        // Outstanding frames remain, so restart timer from this moment.
        timer_start_time = current_time;
    }
}

// Checks sender timer and retransmits outstanding frames on timeout.
void check_timeout(int current_time)
{
    // Skip timeout logic when no outstanding frames exist.
    if (!timer_running)
        return;

    // Return early if timeout duration has not elapsed yet.
    if (current_time - timer_start_time < TIMEOUT)
        return;

    // Log timeout event before retransmission burst.
    cout << "t=" << current_time << ": TIMEOUT -> Retransmitting" << endl;

    // Go-Back-N: retransmit every outstanding frame in the current window.
    for (int i = base; i < next_seq_num; i++)
    {
        // Log each retransmitted frame sequence number.
        cout << "t=" << current_time
             << ": Retransmitting frame " << buffer[i].seq_num << endl;
        // Re-send cached frame through channel.
        send_to_channel(buffer[i]);
    }

    // Keep timer active after retransmission.
    timer_running = true;
    // Restart timer from current simulation time.
    timer_start_time = current_time;
}

// Returns sender window base for simulation completion checks.
int get_sender_base()
{
    // Expose current base value.
    return base;
}

// Returns next sequence number for simulation completion checks.
int get_sender_next_seq()
{
    // Expose current next sequence number value.
    return next_seq_num;
}

// Resets sender module state for a fresh simulation run.
void reset_sender()
{
    // Reset window base.
    base = 0;
    // Reset next outgoing sequence number.
    next_seq_num = 0;
    // Stop timer.
    timer_running = false;
    // Clear timer start timestamp.
    timer_start_time = 0;
}
