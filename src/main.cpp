// =============================================================================
// main.cpp
// -----------------------------------------------------------------------------
// Protocol 5 simulation driver. Wires the sender, receiver, channel, and
// multi-timer modules together and drives a discrete-time simulation loop.
//
// CLI:
//   ./protocol5_sim [frame_loss%] [ack_loss%] [total_frames] [max_time_ms]
//                   [corrupt%] [seed]
//
//   Note: this build models a single bidirectional `frame` type, so the
//   "ack_loss%" argument is interpreted as the loss probability for B->A
//   frames and "frame_loss%" as the loss probability for A->B frames.
//   Internally these are mapped onto a single channel-loss probability that
//   is applied symmetrically: if you want asymmetric loss, set them equal.
//   The two arguments are kept for backward compatibility with the original
//   CLI surface and the larger of the two is used.
//
//   `seed`: pass 0 (or omit) to seed from wall-clock time, or any non-zero
//   value for a reproducible run.
// =============================================================================

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "Header.h"
#include "Channel.h"
#include "Timer.h"

int main(int argc, char* argv[])
{
    double frame_loss_percent = 0.0;
    double ack_loss_percent   = 0.0;
    int    total_frames       = 12;
    int    max_time_ms        = 8000;
    double corrupt_percent    = 0.0;
    unsigned int seed         = 0;

    constexpr int STEP_MS = 25;

    if (argc > 1) frame_loss_percent = std::atof(argv[1]);
    if (argc > 2) ack_loss_percent   = std::atof(argv[2]);
    if (argc > 3) total_frames       = std::atoi(argv[3]);
    if (argc > 4) max_time_ms        = std::atoi(argv[4]);
    if (argc > 5) corrupt_percent    = std::atof(argv[5]);
    if (argc > 6) seed               = static_cast<unsigned int>(std::atoi(argv[6]));

    sender_reset();
    receiver_reset();
    timers_reset();
    channel_reset();
    channel_seed(seed);

    register_a_to_b(receiver_on_frame_arrival);
    register_b_to_a(sender_on_frame_arrival);

    set_frame_loss_probability(std::max(frame_loss_percent, ack_loss_percent));
    set_frame_corrupt_probability(corrupt_percent);

    std::cout << "=== Protocol 5 (Go-Back-N) Simulation ===" << std::endl;
    std::cout << "frame_loss="    << frame_loss_percent
              << "%, ack_loss="   << ack_loss_percent
              << "%, corrupt="    << corrupt_percent
              << "%, frames="     << total_frames
              << ", max_time_ms=" << max_time_ms
              << ", seed="        << seed
              << std::endl;
    std::cout << "MAX_SEQ=" << MAX_SEQ
              << ", WINDOW_SIZE=" << WINDOW_SIZE
              << ", PROP_DELAY="  << PROP_DELAY << "ms"
              << ", TX_TIME="     << TRANSMISSION_TIME << "ms"
              << ", TIMEOUT="     << TIMEOUT_MS << "ms"
              << std::endl << std::endl;

    int  app_packets_offered = 0;
    bool completed           = false;

    for (int t = 0; t <= max_time_ms; t += STEP_MS)
    {
        // network_layer_ready: feed packets while the sender has window room.
        while (app_packets_offered < total_frames && sender_can_accept_packet())
        {
            packet p;
            p.data = app_packets_offered;
            sender_from_network_layer(p, t);
            ++app_packets_offered;
        }

        // frame_arrival events fire from inside process_channel via callbacks.
        process_channel(t);

        // timeout events: pop every expired timer for this tick.
        seq_nr expired = 0;
        while (pop_expired_timer(t, &expired))
            sender_on_timeout(expired, t);

        if (receiver_packets_delivered() == total_frames
            && sender_nbuffered() == 0)
        {
            std::cout << std::endl
                      << "t=" << t << ": ALL " << total_frames
                      << " packets delivered and acknowledged." << std::endl;
            completed = true;
            break;
        }
    }

    if (!completed)
    {
        std::cout << std::endl
                  << "Simulation ended before full completion." << std::endl;
        std::cout << "  packets offered to sender    = " << app_packets_offered << std::endl;
        std::cout << "  packets delivered to net lyr = " << receiver_packets_delivered() << std::endl;
        std::cout << "  sender ack_expected          = " << sender_ack_expected() << std::endl;
        std::cout << "  sender next_frame_to_send    = " << sender_next_frame_to_send() << std::endl;
        std::cout << "  sender nbuffered             = " << sender_nbuffered() << std::endl;
    }

    std::cout << std::endl
              << "Stats: sender transmitted " << sender_total_sent()
              << " frames total (" << total_frames << " unique data packets)"
              << std::endl;

    return completed ? 0 : 1;
}
