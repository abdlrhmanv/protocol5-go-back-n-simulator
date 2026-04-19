// =============================================================================
// Timer.h
// -----------------------------------------------------------------------------
// Multi-timer module modeled after Fig. 3.20 in Tanenbaum & Wetherall.
// Each outstanding frame has its own timer; entries are kept sorted by
// expiration so the simulation driver can pop expired timers in order.
// =============================================================================

#ifndef TIMER_H
#define TIMER_H

#include "Header.h"

// Start (or restart) the timer for a given sequence number. A subsequent call
// with the same `seq` replaces the previous expiry.
void start_timer(seq_nr seq, int current_time);

// Cancel the timer for `seq`, if any.
void stop_timer(seq_nr seq);

// If any timer has expired (expiry <= current_time), removes and returns it
// via *expired_seq and returns true. Returns false if no timer is due.
//
// Call repeatedly each tick until it returns false.
bool pop_expired_timer(int current_time, seq_nr* expired_seq);

// Forget all pending timers (used when resetting the simulation).
void timers_reset();

#endif
