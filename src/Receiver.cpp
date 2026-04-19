// =============================================================================
// Receiver.cpp
// -----------------------------------------------------------------------------
// Receiver side of Protocol 5 (Go-Back-N). Mirrors the frame_arrival and
// cksum_err handling in Tanenbaum's protocol5().
//
// Notes on ACK strategy (and how it differs from a strict Protocol 5):
//
//   In Tanenbaum's Protocol 5 there is no separate "ACK" frame: the receiver
//   always has reverse data of its own, so it piggybacks the ACK on outbound
//   data. Our simulation has only one direction of data flow, so the receiver
//   sends an "ACK-only" frame (kind = FK_ACK) every time it accepts an
//   in-order frame. If a real bidirectional implementation were used this
//   would simply be the `ack` field on a regular DATA frame.
//
// Bug fixed vs. previous version: when no in-order frame has yet been
// received, the receiver no longer sends a bogus ACK referring to
// (expected_seq_num - 1) which would wrap to MAX_SEQ and trick the sender
// into believing that everything was acknowledged.
// =============================================================================

#include <iostream>
#include "Header.h"
#include "Channel.h"

namespace
{
seq_nr frame_expected     = 0;       // next in-order seq we will accept
bool   any_frame_accepted = false;   // false until first in-order frame
int    delivered_count    = 0;       // packets passed to the network layer

void send_ack(seq_nr ack_num, int current_time)
{
    frame s;
    s.kind     = FK_ACK;
    s.seq      = MAX_SEQ;       // unused for ACK-only frames
    s.ack      = ack_num;
    s.info     = packet{0};
    s.corrupt  = false;

    std::cout << "t=" << current_time
              << ": RECVR   send ACK ack=" << ack_num << std::endl;

    send_to_channel(s, DIR_BA, current_time);
}
}

void receiver_reset()
{
    frame_expected     = 0;
    any_frame_accepted = false;
    delivered_count    = 0;
}

void receiver_on_frame_arrival(frame f, int current_time)
{
    // cksum_err: just ignore bad frames (textbook GBN).
    if (f.corrupt)
    {
        std::cout << "t=" << current_time
                  << ": RECVR   cksum_err -- frame discarded, no ACK"
                  << std::endl;
        return;
    }

    if (f.kind != FK_DATA)
    {
        // Receiver should not see ACK-only frames; harmless to ignore.
        return;
    }

    if (f.seq == frame_expected)
    {
        std::cout << "t=" << current_time
                  << ": RECVR   accepted DATA seq=" << f.seq
                  << " (payload=" << f.info.data
                  << ") -> network layer" << std::endl;

        ++delivered_count;
        send_ack(frame_expected, current_time);
        frame_expected     = inc_seq(frame_expected);
        any_frame_accepted = true;
    }
    else
    {
        std::cout << "t=" << current_time
                  << ": RECVR   out-of-order DATA seq=" << f.seq
                  << " (expected " << frame_expected << ") -- discarded"
                  << std::endl;

        // Re-ACK the last correctly received frame so the sender retransmits
        // promptly. If we have not yet accepted anything, do not send a fake
        // ACK -- silence is correct because the sender's timer will fire.
        if (any_frame_accepted)
        {
            seq_nr last_good = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);
            send_ack(last_good, current_time);
        }
    }
}

int receiver_packets_delivered()
{
    return delivered_count;
}
