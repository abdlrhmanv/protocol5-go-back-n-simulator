// =============================================================================
// Channel.h
// -----------------------------------------------------------------------------
// Public API for channel simulation layer.
//
// PURPOSE
//   - Declares callback signatures and channel control functions.
//   - Defines the bridge between sender/receiver and channel internals.
//
// TEAM CONTRACT
//   - Keep this API stable for all modules.
//   - Do not add simulation-driver dependencies here.
//   - If callback signatures change, update registration and implementations.
// =============================================================================

#ifndef CHANNEL_H  // Starts include guard for this header.
#define CHANNEL_H  // Defines include guard macro.

#include "Header.h"  // Imports Frame/Ack definitions shared across modules.

// Function pointer type for delivering one frame to receiver logic.
typedef void (*FrameDeliveryCallback)(Frame, int);

// Function pointer type for delivering one ACK to sender logic.
typedef void (*AckDeliveryCallback)(int, int);

// Registers the receiver callback used when frame delivery time expires.
void register_receiver(FrameDeliveryCallback cb);

// Registers the sender callback used when ACK delivery time expires.
void register_sender_ack(AckDeliveryCallback cb);

// Sets frame loss probability percentage used by channel model.
void set_frame_loss_probability(double percent);
// Sets ACK loss probability percentage used by channel model.
void set_ack_loss_probability(double percent);

// Updates channel's current simulation time.
void set_channel_time(int current_time);

// Accepts one sender frame and schedules it for delayed delivery/drop.
void send_to_channel(Frame f);

// Accepts one receiver ACK and schedules it for delayed delivery/drop.
void send_ack_to_channel(Ack a);

// Processes frame/ACK queues and delivers all packets due at this time.
void process_channel(int current_time);

#endif  // Ends include guard.
