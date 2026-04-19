// =============================================================================
// Timer.cpp
// -----------------------------------------------------------------------------
// Multi-timer implementation per Fig. 3.20: a list of (seq, expiry) entries
// kept sorted by expiry. Insertion / removal is O(N) but N <= MAX_SEQ so this
// is more than fast enough for an educational simulation.
// =============================================================================

#include <list>
#include "Timer.h"

namespace
{
struct TimerEntry
{
    seq_nr seq;
    int    expiry_ms;
};

std::list<TimerEntry> g_timers;
}

void start_timer(seq_nr seq, int current_time)
{
    stop_timer(seq);

    TimerEntry e{seq, current_time + TIMEOUT_MS};

    auto it = g_timers.begin();
    while (it != g_timers.end() && it->expiry_ms <= e.expiry_ms)
        ++it;
    g_timers.insert(it, e);
}

void stop_timer(seq_nr seq)
{
    for (auto it = g_timers.begin(); it != g_timers.end(); ++it)
    {
        if (it->seq == seq)
        {
            g_timers.erase(it);
            return;
        }
    }
}

bool pop_expired_timer(int current_time, seq_nr* expired_seq)
{
    if (g_timers.empty())
        return false;
    if (g_timers.front().expiry_ms > current_time)
        return false;

    *expired_seq = g_timers.front().seq;
    g_timers.pop_front();
    return true;
}

void timers_reset()
{
    g_timers.clear();
}
