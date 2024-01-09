// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Use a large enough redzone value to avoid stack overflow between
/// calls to stack_limit::maybe_grow(). This can be adjusted but
/// should be big enough to accommodate debug builds. HHVM's equivalent
/// value is RequestInfo::StackSlack, currently set to 1MiB.
const RED_ZONE: usize = 128 * 1024; // 128KiB

/// When the stack grows, allocate a new stack segment of this size.
const STACK_SIZE: usize = 16 * RED_ZONE; // 2MiB

/// Lightweight stack size tracking facility
///
/// # Usage:
/// ```
/// {
///   // casual recursion: no checks needed within RED_ZONE.
///   // Can also reset & check peak stack depth.
///   stack_limit::reset();
///   let x = stack_limit::maybe_grow(move || {
///     // fearless recursion: stack will be at least STACK_SIZE
///     // if remaining space is below RED_ZONE bytes.
///   });
///   println!("max depth {}", stack_limit::peak());
///   x
/// }
///
/// Call stack_limit::maybe_grow() along recursive paths that may otherwise
/// overflow. The called lambda continues to run on the same thread even
/// when the stack is grown.
/// ```

pub fn maybe_grow<T>(f: impl FnOnce() -> T) -> T {
    let sp = psm::stack_pointer();
    match stacker::remaining_stack() {
        Some(r) if r < RED_ZONE => {
            // Save old base & depth values before growing stack, and update peak.
            let old = TRACKER.with(|t| {
                let old = t.get();
                let depth = old.depth(sp);
                t.replace(Tracker {
                    depth,
                    peak: std::cmp::max(old.peak, depth),
                    ..old
                })
            });
            let x = stacker::grow(STACK_SIZE, || {
                // Get the new base stack pointer.
                TRACKER.with(|t| {
                    t.set(Tracker {
                        base: psm::stack_pointer(),
                        ..t.get()
                    })
                });
                f()
            });
            TRACKER.with(|t| {
                // Restore old tracker but preserve peak.
                t.set(Tracker {
                    peak: t.get().peak,
                    ..old
                })
            });
            return x;
        }
        Some(_) => {
            // No need to grow, just update peak.
            TRACKER.with(|t| {
                let old = t.get();
                t.set(Tracker {
                    peak: std::cmp::max(old.peak, old.depth(sp)),
                    ..old
                })
            });
        }
        None => {}
    }
    f()
}

#[derive(Debug, Clone, Copy)]
struct Tracker {
    /// Highest address in current stack segment.
    base: *const u8,

    /// Maximum stack depth since last reset().
    peak: usize,

    /// Stack depth just before most recent grow().
    depth: usize,
}

impl Default for Tracker {
    fn default() -> Self {
        Self {
            base: std::ptr::null(),
            peak: 0,
            depth: 0,
        }
    }
}

impl Tracker {
    fn depth(&self, sp: *const u8) -> usize {
        self.depth + (self.base as usize).abs_diff(sp as usize)
    }
}

pub fn reset() {
    let base = psm::stack_pointer();
    TRACKER.with(|t| {
        t.set(Tracker {
            base,
            ..Default::default()
        });
    });
}

pub fn peak() -> usize {
    TRACKER.with(|t| t.get().peak)
}

thread_local! {
    static TRACKER: std::cell::Cell<Tracker> = Default::default();
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;

    fn detect_growth() -> bool {
        TRACKER.with(|t| t.get().depth != 0)
    }

    // Ackerman function that returns a K-sized array of result values
    // to consume stack space faster.
    #[inline(never)]
    fn ackermann<const K: usize>(m: i64, n: i64) -> ([i64; K], bool) {
        let over = detect_growth();
        if m == 0 {
            return ([n + 1; K], over);
        }
        maybe_grow(|| {
            if n == 0 {
                let (a, inner_over) = ackermann(m - 1, 1);
                (a, over | inner_over)
            } else {
                let (a1, over1) = ackermann::<K>(m, n - 1);
                let (a2, over2) = ackermann::<K>(m - 1, a1[0]);
                (a2, over | over1 | over2)
            }
        })
    }

    // Find the lowest value of n that will encroach on RED_ZONE
    fn min_n_that_overflows<const K: usize>(m: i64) -> i64 {
        for n in 2..8 {
            eprintln!("trying ({},{})", m, n);
            if let (_, true) = ackermann::<K>(m, n) {
                eprintln!("overflow at n={}", n);
                return n;
            }
        }
        0
    }

    fn ackermann_test<const K: usize>() -> bool {
        const M: i64 = 3;
        let n = min_n_that_overflows::<K>(M);
        if n < 4 {
            eprintln!("K={} n={} rejected", K, n);
            return false;
        }
        // Ackermann recursion depth grows ~2x when n is increased by 1 and m is fixed,
        // so this should trigger stacker::grow() but not panic or crash.
        assert!(matches!(ackermann::<K>(M, n + 1), (_, true)));
        true
    }

    #[test]
    fn test() {
        assert!(!detect_growth());
        // Try to find a good value of K (stack bloat for each stack frame)
        // so the test recurses deep enough but also terminates reasonably fast.
        if ackermann_test::<1000>() {
            return;
        }
        if ackermann_test::<500>() {
            return;
        }
        if ackermann_test::<200>() {
            return;
        }
        if ackermann_test::<100>() {
            return;
        }
        if ackermann_test::<50>() {
            return;
        }
        panic!();
    }
}
