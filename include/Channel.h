// =============================================================================
// Channel.h
// -----------------------------------------------------------------------------
// Bidirectional lossy/corrupting/delaying channel between sender (A) and
// receiver (B). Carries the unified `frame` type in either direction.
// =============================================================================

#ifndef CHANNEL_H
#define CHANNEL_H

#include "Header.h"

// Direction of travel for a frame handed to the channel.
enum direction
{
    DIR_AB,  // A (sender) -> B (receiver): a data frame
    DIR_BA   // B (receiver) -> A (sender): an ack-only or piggybacked frame
};

// Callback signature used to deliver a frame to the protocol layer.
typedef void (*FrameDeliveryCallback)(frame, int);

// Register the receiver's frame_arrival handler (A -> B direction).
void register_a_to_b(FrameDeliveryCallback cb);

// Register the sender's frame_arrival handler (B -> A direction).
void register_b_to_a(FrameDeliveryCallback cb);

// Probability (0..100) that the channel silently drops a frame.
void set_frame_loss_probability(double percent);

// Probability (0..100) that the channel corrupts a frame. A corrupted frame
// is delivered as the same frame structure but flagged so the protocol layer
// can model a `cksum_err` event (per Tanenbaum, GBN ignores bad frames).
void set_frame_corrupt_probability(double percent);

// Seed the channel's PRNG for reproducible simulations. Pass 0 to seed from
// wall-clock time.
void channel_seed(unsigned int seed);

// Hand a frame to the channel for delivery. The channel logs acceptance,
// applies loss/corruption, and schedules delivery after TRANSMISSION_TIME +
// PROP_DELAY ms.
void send_to_channel(frame f, direction d, int current_time);

// Deliver every frame whose scheduled delivery time has been reached.
void process_channel(int current_time);

// Discard all in-flight frames and reset PRNG-independent state.
void channel_reset();

#endif
