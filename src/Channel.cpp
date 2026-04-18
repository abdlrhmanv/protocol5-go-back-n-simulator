// =============================================================================
// channel.cpp
// -----------------------------------------------------------------------------
// Lossy/delayed channel model that transports data frames and ACKs.
// =============================================================================

#include <iostream>   // Provides std::cout/std::endl for channel trace logs.
#include <vector>     // Provides std::vector for in-flight packet queues.
#include <cstdlib>    // Provides rand() and srand() for loss simulation.
#include <ctime>      // Provides time() for one-time random seed.
#include "Channel.h"  // Channel interface and callback type declarations.
#include "Header.h"   // Shared Frame/Ack definitions and constants.

using namespace std;  // Keeps implementation concise for coursework context.

// Represents a data frame currently traveling through the channel.
struct InFlightFrame
{
    // Payload frame object.
    Frame frame;
    // Simulation time when this frame should be delivered.
    int delivery_time;
};

// Represents an ACK currently traveling through the channel.
struct InFlightAck
{
    // ACK payload object.
    Ack ack;
    // Simulation time when this ACK should be delivered.
    int delivery_time;
};

// Queue of data frames waiting for delivery time.
static vector<InFlightFrame> frame_queue;
// Queue of ACKs waiting for delivery time.
static vector<InFlightAck> ack_queue;

// Callback invoked when a frame reaches receiver side.
static FrameDeliveryCallback receiver_callback = nullptr;
// Callback invoked when an ACK reaches sender side.
static AckDeliveryCallback sender_ack_callback = nullptr;

// Channel's notion of current simulation time.
static int channel_current_time = 0;
// Probability (percentage) of dropping a data frame.
static double frame_loss_probability = 0.0;
// Probability (percentage) of dropping an ACK.
static double ack_loss_probability = 0.0;

// Flag to avoid reseeding pseudo-random generator repeatedly.
static bool random_initialized = false;

// Seeds PRNG once using wall-clock time.
static void initialize_random_once()
{
    // Only seed if not already seeded.
    if (!random_initialized)
    {
        // Initialize pseudo-random generator seed.
        srand((unsigned int)time(nullptr));
        // Mark initialization completed.
        random_initialized = true;
    }
}

// Returns true if packet should be dropped for a given percentage.
static bool is_lost(double percent)
{
    // Ensure random generator is seeded before sampling.
    initialize_random_once();
    // Generate pseudo-random value in [0.00, 99.99].
    double r = (rand() % 10000) / 100.0;
    // Decide drop if sample falls below requested percentage.
    return (r < percent);
}

// Registers receiver callback for frame deliveries.
void register_receiver(FrameDeliveryCallback cb)
{
    // Save receiver callback pointer.
    receiver_callback = cb;
}

// Registers sender callback for ACK deliveries.
void register_sender_ack(AckDeliveryCallback cb)
{
    // Save sender callback pointer.
    sender_ack_callback = cb;
}

// Configures frame drop probability.
void set_frame_loss_probability(double percent)
{
    // Store frame loss percentage.
    frame_loss_probability = percent;
}

// Configures ACK drop probability.
void set_ack_loss_probability(double percent)
{
    // Store ACK loss percentage.
    ack_loss_probability = percent;
}

// Updates channel clock reference used for enqueue timestamps.
void set_channel_time(int current_time)
{
    // Record current simulation time.
    channel_current_time = current_time;
}

// Enqueues one frame for delayed delivery (or probabilistic drop).
void send_to_channel(Frame f)
{
    // Apply frame loss model before enqueueing.
    if (is_lost(frame_loss_probability))
    {
        // Log dropped frame event.
        cout << "t=" << channel_current_time
             << ": CHANNEL lost frame " << f.seq_num << endl;
        // Abort enqueue for dropped frame.
        return;
    }

    // Prepare queue element for in-flight frame.
    InFlightFrame item;
    // Copy frame payload into queue element.
    item.frame = f;
    // Compute target delivery time using propagation delay.
    item.delivery_time = channel_current_time + PROP_DELAY;
    // Push item into in-flight frame queue.
    frame_queue.push_back(item);

    // Log accepted frame and expected delivery timestamp.
    cout << "t=" << channel_current_time
         << ": CHANNEL accepted frame " << f.seq_num
         << ", will reach receiver at t=" << item.delivery_time << endl;
}

// Enqueues one ACK for delayed delivery (or probabilistic drop).
void send_ack_to_channel(Ack a)
{
    // Apply ACK loss model before enqueueing.
    if (is_lost(ack_loss_probability))
    {
        // Log dropped ACK event.
        cout << "t=" << channel_current_time
             << ": CHANNEL lost ACK " << a.ack_num << endl;
        // Abort enqueue for dropped ACK.
        return;
    }

    // Prepare queue element for in-flight ACK.
    InFlightAck item;
    // Copy ACK payload into queue element.
    item.ack = a;
    // Compute target delivery time using propagation delay.
    item.delivery_time = channel_current_time + PROP_DELAY;
    // Push item into in-flight ACK queue.
    ack_queue.push_back(item);

    // Log accepted ACK and expected delivery timestamp.
    cout << "t=" << channel_current_time
         << ": CHANNEL accepted ACK " << a.ack_num
         << ", will reach sender at t=" << item.delivery_time << endl;
}

// Delivers all due frames/ACKs whose delivery_time <= current_time.
void process_channel(int current_time)
{
    // Synchronize channel's local clock with simulation time.
    channel_current_time = current_time;

    // Scan frame queue and deliver any frame whose delay has expired.
    for (int i = 0; i < (int)frame_queue.size(); )
    {
        // Check whether current queued frame is due now.
        if (frame_queue[i].delivery_time <= current_time)
        {
            // Copy due frame payload.
            Frame f = frame_queue[i].frame;
            // Log frame delivery event.
            cout << "t=" << current_time
                 << ": CHANNEL delivered frame " << f.seq_num
                 << " to receiver" << endl;

            // Invoke receiver callback if it has been registered.
            if (receiver_callback != nullptr)
                receiver_callback(f, current_time);

            // Remove delivered frame from queue without incrementing i.
            frame_queue.erase(frame_queue.begin() + i);
        }
        else
        {
            // Move to next queued frame when current one is not due yet.
            i++;
        }
    }

    // Scan ACK queue and deliver any ACK whose delay has expired.
    for (int i = 0; i < (int)ack_queue.size(); )
    {
        // Check whether current queued ACK is due now.
        if (ack_queue[i].delivery_time <= current_time)
        {
            // Copy due ACK payload.
            Ack a = ack_queue[i].ack;
            // Log ACK delivery event.
            cout << "t=" << current_time
                 << ": CHANNEL delivered ACK " << a.ack_num
                 << " to sender" << endl;

            // Invoke sender callback if it has been registered.
            if (sender_ack_callback != nullptr)
                sender_ack_callback(a.ack_num, current_time);

            // Remove delivered ACK from queue without incrementing i.
            ack_queue.erase(ack_queue.begin() + i);
        }
        else
        {
            // Move to next queued ACK when current one is not due yet.
            i++;
        }
    }
}
