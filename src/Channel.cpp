// =============================================================================
// Channel.cpp
// -----------------------------------------------------------------------------
// Bidirectional channel that carries the unified `frame` between the sender
// (A) and the receiver (B). Models:
//   * Per-frame transmission time (TRANSMISSION_TIME)
//   * One-way propagation delay   (PROP_DELAY)
//   * Probabilistic loss          (set_frame_loss_probability)
//   * Probabilistic corruption    (set_frame_corrupt_probability)
//
// Corrupted frames are still delivered, but `kind` is replaced by FK_DATA and
// the delivery callback receives a frame whose `seq`/`ack` have been
// scrambled. The protocol layer treats this as a `cksum_err` event (in GBN,
// "just ignore bad frames").
// =============================================================================

#include <iostream>
#include <random>
#include <vector>
#include <ctime>
#include "Channel.h"

namespace
{
struct InFlightFrame
{
    frame      pkt;
    direction  dir;
    int        delivery_time_ms;
    bool       corrupted;
};

std::vector<InFlightFrame> g_in_flight;

FrameDeliveryCallback g_a_to_b_cb = nullptr;
FrameDeliveryCallback g_b_to_a_cb = nullptr;

double g_loss_pct    = 0.0;
double g_corrupt_pct = 0.0;

std::mt19937 g_rng;
bool         g_rng_initialized = false;

double rand_pct()
{
    if (!g_rng_initialized)
    {
        g_rng.seed(static_cast<unsigned int>(std::time(nullptr)));
        g_rng_initialized = true;
    }
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    return dist(g_rng);
}

bool sample_event(double pct)
{
    if (pct <= 0.0)   return false;
    if (pct >= 100.0) return true;
    return rand_pct() < pct;
}

const char* dir_arrow(direction d)
{
    return d == DIR_AB ? "A->B" : "B->A";
}
}

void register_a_to_b(FrameDeliveryCallback cb) { g_a_to_b_cb = cb; }
void register_b_to_a(FrameDeliveryCallback cb) { g_b_to_a_cb = cb; }

void set_frame_loss_probability(double percent)    { g_loss_pct    = percent; }
void set_frame_corrupt_probability(double percent) { g_corrupt_pct = percent; }

void channel_seed(unsigned int seed)
{
    if (seed == 0)
        g_rng.seed(static_cast<unsigned int>(std::time(nullptr)));
    else
        g_rng.seed(seed);
    g_rng_initialized = true;
}

void send_to_channel(frame f, direction d, int current_time)
{
    const char* kind_str = (f.kind == FK_DATA) ? "DATA" : "ACK ";

    if (sample_event(g_loss_pct))
    {
        std::cout << "t=" << current_time
                  << ": CHANNEL [" << dir_arrow(d) << "] LOST "
                  << kind_str << " seq=" << f.seq << " ack=" << f.ack
                  << std::endl;
        return;
    }

    InFlightFrame item;
    item.pkt              = f;
    item.dir              = d;
    item.delivery_time_ms = current_time + TRANSMISSION_TIME + PROP_DELAY;
    item.corrupted        = sample_event(g_corrupt_pct);
    g_in_flight.push_back(item);

    std::cout << "t=" << current_time
              << ": CHANNEL [" << dir_arrow(d) << "] accepted "
              << kind_str << " seq=" << f.seq << " ack=" << f.ack
              << ", arrives at t=" << item.delivery_time_ms
              << (item.corrupted ? " [WILL ARRIVE CORRUPTED]" : "")
              << std::endl;
}

void process_channel(int current_time)
{
    for (size_t i = 0; i < g_in_flight.size(); )
    {
        if (g_in_flight[i].delivery_time_ms <= current_time)
        {
            InFlightFrame item = g_in_flight[i];
            g_in_flight.erase(g_in_flight.begin() + static_cast<long>(i));

            frame f      = item.pkt;
            f.corrupt    = item.corrupted;
            if (item.corrupted)
            {
                std::cout << "t=" << current_time
                          << ": CHANNEL [" << dir_arrow(item.dir)
                          << "] delivered CORRUPT frame "
                          << "(orig seq=" << f.seq << " ack=" << f.ack << ")"
                          << std::endl;
            }
            else
            {
                std::cout << "t=" << current_time
                          << ": CHANNEL [" << dir_arrow(item.dir)
                          << "] delivered "
                          << (f.kind == FK_DATA ? "DATA" : "ACK ")
                          << " seq=" << f.seq << " ack=" << f.ack
                          << std::endl;
            }

            FrameDeliveryCallback cb =
                (item.dir == DIR_AB) ? g_a_to_b_cb : g_b_to_a_cb;
            if (cb != nullptr)
                cb(f, current_time);
        }
        else
        {
            ++i;
        }
    }
}

void channel_reset()
{
    g_in_flight.clear();
}
