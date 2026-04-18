// =============================================================================
// receiver.cpp
// -----------------------------------------------------------------------------
// Receiver-side implementation for Protocol 5.
// =============================================================================

#include <iostream>   // Provides std::cout and std::endl for event tracing.
#include "Header.h"   // Shared Frame/Ack models and receiver declarations.
#include "Channel.h"  // Channel API used to send ACKs.

using namespace std;  // Keeps educational sample concise.

// Stores the next in-order sequence number expected by the receiver.
int expected_seq_num = 0;

// Handles one incoming frame and returns cumulative ACK behavior.
void receive_frame(Frame frame, int current_time)
{
    // Log frame arrival at receiver side.
    cout << "t=" << current_time
         << ": Receiver got frame " << frame.seq_num << endl;

    // Prepare ACK object that will be sent back after decision.
    Ack ack;

    // Accept only strictly in-order frame.
    if (frame.seq_num == expected_seq_num)
    {
        // Log successful in-order acceptance.
        cout << "t=" << current_time
             << ": Receiver accepted frame " << frame.seq_num << endl;

        // Cumulative ACK acknowledges this newly accepted frame.
        ack.ack_num = frame.seq_num;
        // Advance expected sequence number with wraparound support.
        expected_seq_num = (expected_seq_num + 1) % MAX_SEQ;
    }
    else
    {
        // Log out-of-order/discarded frame.
        cout << "t=" << current_time
             << ": Receiver discarded frame " << frame.seq_num << endl;

        // Re-ACK last correctly received in-order frame.
        ack.ack_num = (expected_seq_num - 1 + MAX_SEQ) % MAX_SEQ;
    }

    // Send generated ACK back via channel simulator.
    send_ack_to_channel(ack);
}

// Resets receiver state before each independent simulation run.
void reset_receiver()
{
    // Restore expected sequence number to initial state.
    expected_seq_num = 0;
}
