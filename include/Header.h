// =============================================================================
// Header.h
// -----------------------------------------------------------------------------
// Shared protocol model + public sender/receiver interfaces for the
// Protocol 5 (Go-Back-N) simulation.
//
// Naming and constants follow Tanenbaum & Wetherall, "Computer Networks", 5e,
// Fig. 3.19 / Fig. 3.20.
// =============================================================================

#ifndef HEADER_H
#define HEADER_H

// Highest legal sequence number. The sequence-number space has size MAX_SEQ+1
// (= 8, i.e. 3 bits) which matches the textbook's #define MAX_SEQ 7.
constexpr int MAX_SEQ = 7;

// For Go-Back-N the sender window may hold up to MAX_SEQ outstanding frames.
constexpr int WINDOW_SIZE = MAX_SEQ;

// Number of slots in the sender's circular retransmission buffer.
constexpr int NR_BUFS = MAX_SEQ + 1;

// One-way propagation delay (ms). Applies to data and ACK frames.
constexpr int PROP_DELAY = 200;

// Per-frame serialization / transmission time on the wire (ms). Without this
// every frame in a window would leave at the same instant and the trace would
// not show pipelining behavior.
constexpr int TRANSMISSION_TIME = 30;

// Sender retransmission timer duration (ms). Must exceed RTT + receiver work.
constexpr int TIMEOUT_MS = 1000;

// Logical sequence-number type.
using seq_nr = int;

// Tanenbaum's Protocol 5 has only one frame "kind" (data) because ACKs are
// always piggybacked on reverse data traffic. In our simulation the receiver
// has no reverse traffic of its own, so we add an ACK-only kind that models
// the limit case of Protocol 5 (an empty data frame whose only useful field
// is `ack`). With bidirectional traffic both sides would always send DATA.
enum frame_kind { FK_DATA, FK_ACK };

// Application-layer payload carried inside a data frame.
struct packet
{
    int data;
};

// Wire format of a frame. Mirrors Tanenbaum's `frame` struct.
//
// `corrupt` is a simulation-only flag (not part of any real wire format) used
// by the channel to signal a checksum failure to the protocol layer. The
// protocol layer treats `corrupt == true` exactly like the textbook's
// `cksum_err` event.
struct frame
{
    frame_kind kind;     // DATA or ACK-only
    seq_nr     seq;      // sequence number of this frame
    seq_nr     ack;      // piggybacked cumulative ack (n means "n received")
    packet     info;     // network-layer packet (only meaningful for FK_DATA)
    bool       corrupt;  // sim-only: true => deliver as cksum_err
};

// Returns true iff a <= b < c circularly. Direct port of the textbook helper
// used to test "frame b lies inside the open window [a, c)".
bool between(seq_nr a, seq_nr b, seq_nr c);

// Increments a sequence number modulo (MAX_SEQ + 1).
seq_nr inc_seq(seq_nr s);

// ============================ Sender API =====================================

// Reset all sender state for a fresh run.
void sender_reset();

// True iff the sender's window has room for one more outstanding frame.
bool sender_can_accept_packet();

// Network-layer-ready event: hand a fresh packet to the sender.
void sender_from_network_layer(packet p, int current_time);

// Frame-arrival event: an ACK (or piggybacked ACK) reached the sender.
void sender_on_frame_arrival(frame f, int current_time);

// Timeout event for a specific outstanding sequence number (Fig. 3.20).
void sender_on_timeout(seq_nr seq, int current_time);

// Number of frames currently in the sender window awaiting ACK.
int sender_nbuffered();

// Sequence number of the oldest unacknowledged frame.
seq_nr sender_ack_expected();

// Next sequence number the sender will assign to a new frame.
seq_nr sender_next_frame_to_send();

// Total number of frames the sender has transmitted (originals + retransmits).
unsigned long sender_total_sent();

// =========================== Receiver API ====================================

// Reset all receiver state for a fresh run.
void receiver_reset();

// Frame-arrival event at the receiver.
void receiver_on_frame_arrival(frame f, int current_time);

// Number of in-order packets the receiver has handed to its network layer.
int receiver_packets_delivered();

#endif
