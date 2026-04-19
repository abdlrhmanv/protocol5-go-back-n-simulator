// =============================================================================
// Sender.cpp
// -----------------------------------------------------------------------------
// Sender side of Protocol 5 (Go-Back-N), structured to mirror the textbook's
// Fig. 3.19 implementation:
//
//   - All sequence-number arithmetic is modulo (MAX_SEQ + 1).
//   - The window check uses the helper `between()`.
//   - Outgoing packets are kept in a circular buffer of NR_BUFS slots and
//     indexed by `seq % NR_BUFS`.
//   - Each outstanding frame has its own timer (Fig. 3.20). On a timeout for
//     the oldest frame the sender retransmits all frames in [ack_expected,
//     next_frame_to_send) -- the GBN "go back to N" behavior.
//   - Bad frames are silently ignored (the textbook GBN cksum_err handler).
// =============================================================================

#include <iostream>
#include "Header.h"
#include "Channel.h"
#include "Timer.h"

namespace
{
seq_nr  ack_expected       = 0;   // oldest frame as yet unacknowledged
seq_nr  next_frame_to_send = 0;   // next frame going out
int     nbuffered          = 0;   // number of output buffers currently in use

packet  buffer[NR_BUFS];          // circular buffer of outbound packets
unsigned long total_sent_count = 0;

void send_data(seq_nr frame_nr, int current_time)
{
    frame s;
    s.kind    = FK_DATA;
    s.seq     = frame_nr;
    // No reverse data flow in this scenario, so we leave the piggybacked ack
    // field meaningless (set to MAX_SEQ as a never-valid sentinel).
    s.ack     = MAX_SEQ;
    s.info    = buffer[frame_nr % NR_BUFS];
    s.corrupt = false;

    std::cout << "t=" << current_time
              << ": SENDER  send DATA seq=" << s.seq
              << " (payload=" << s.info.data << ")" << std::endl;

    send_to_channel(s, DIR_AB, current_time);
    start_timer(frame_nr, current_time);
    ++total_sent_count;
}
}

void sender_reset()
{
    ack_expected       = 0;
    next_frame_to_send = 0;
    nbuffered          = 0;
    total_sent_count   = 0;
    for (int i = 0; i < NR_BUFS; ++i)
        buffer[i].data = 0;
}

bool sender_can_accept_packet()
{
    return nbuffered < WINDOW_SIZE;
}

void sender_from_network_layer(packet p, int current_time)
{
    // Mirrors the network_layer_ready case in protocol5().
    buffer[next_frame_to_send % NR_BUFS] = p;
    nbuffered = nbuffered + 1;
    send_data(next_frame_to_send, current_time);
    next_frame_to_send = inc_seq(next_frame_to_send);
}

void sender_on_frame_arrival(frame f, int current_time)
{
    // cksum_err: just ignore bad frames (textbook GBN).
    if (f.corrupt)
    {
        std::cout << "t=" << current_time
                  << ": SENDER  cksum_err on inbound frame -- ignored"
                  << std::endl;
        return;
    }

    // Cumulative ACK: ack n implies n-1, n-2, ... are also acked.
    while (between(ack_expected, f.ack, next_frame_to_send))
    {
        nbuffered = nbuffered - 1;
        stop_timer(ack_expected);
        std::cout << "t=" << current_time
                  << ": SENDER  ACK " << ack_expected << " accepted"
                  << " (window slides; nbuffered=" << (nbuffered) << ")"
                  << std::endl;
        ack_expected = inc_seq(ack_expected);
    }
}

void sender_on_timeout(seq_nr seq, int current_time)
{
    // Tanenbaum's GBN timeout handler: retransmit every outstanding frame.
    std::cout << "t=" << current_time
              << ": SENDER  TIMEOUT on seq=" << seq
              << " -- retransmitting "
              << nbuffered << " frame(s) starting at seq=" << ack_expected
              << std::endl;

    seq_nr s = ack_expected;
    for (int i = 0; i < nbuffered; ++i)
    {
        send_data(s, current_time);
        s = inc_seq(s);
    }
}

int    sender_nbuffered()           { return nbuffered;          }
seq_nr sender_ack_expected()        { return ack_expected;       }
seq_nr sender_next_frame_to_send()  { return next_frame_to_send; }
unsigned long sender_total_sent()   { return total_sent_count;   }

bool between(seq_nr a, seq_nr b, seq_nr c)
{
    // Direct port of the textbook helper. True iff a <= b < c circularly.
    return ((a <= b) && (b < c))
        || ((c <  a) && (a <= b))
        || ((b <  c) && (c <  a));
}

seq_nr inc_seq(seq_nr s)
{
    return (s + 1) % (MAX_SEQ + 1);
}
