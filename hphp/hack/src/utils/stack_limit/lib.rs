// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod retry;

use std::sync::atomic::{AtomicBool, Ordering};

use detail::*;

/// Lightweight stack size tracking facility
///
/// # Usage:
/// ```
/// use crate::stack_limit::{init, StackLimit, MI};
/// init();  // important: initialize if needed (cheap to call multiple times)
///
/// let limit = std::sync::Arc::new(StackLimit::relative(3_000_000));
/// limit.reset(); // set the baseline (when the stack is low)
/// let limit_ref = limit.clone();
/// let deeply_recursive_task = move || {
///      if limit_ref.check_exceeded() {
///          // abort early
///      }
///  };
///  deeply_recursive_task();  // detect if it would consume >3MB of stack
///  if limit.exceeded() {
///      // handle stack overflow preemptively (e.g., retry with custom stack space)
///      std::thread::Builder::new().stack_size(16 * MI) // 16 MiB
///          .spawn(deeply_recursive_task)
///          .expect("ERROR: thread::spawn")
///          .join();
///  }
///  ```
/// *Notes*:
/// - each thread keeps its own stack usage info (thread-safely but lock-free for efficiency)
/// - you need to call `check_exceeded` from each thread that may exceed its stack space
///   (a separate StackLimit instance for each group with different limit is sufficient)
/// - if you decrease the limit for the same thread, you need to call `reset` beforehand
///   (this is a design choice; doing it automatically upon initialization would risk
///   silently missing stack overflows if you re-create an instance on the same thread!)
#[derive(Debug)]
pub struct StackLimit {
    value: usize,
    overflow: AtomicBool,
}

/// Kibi unit (2^10) - a stack size is usually a multiple of it
pub const KI: usize = 1024;
// Mebi unit (2^20)
pub const MI: usize = KI * KI;
// Gibi unit (2^30)
pub const GI: usize = KI * MI;

impl StackLimit {
    pub fn relative(value: usize) -> Self {
        Self {
            value,
            overflow: AtomicBool::new(false),
        }
    }

    pub fn get(&self) -> usize {
        self.value
    }

    pub fn set(&mut self, value: usize) {
        self.value = value
    }

    pub fn check_exceeded(&self) -> bool {
        let overflow = StackGuard::exceeds_size(self.value);
        if overflow {
            // Note: we never store false, so Release constraint (write reordering suffices)
            self.overflow.store(true, Ordering::Release);
        }
        overflow
    }

    pub fn panic_if_exceeded(&self) {
        if self.check_exceeded() {
            panic!("stack overflow prevented by StackLimit");
        }
    }

    pub fn exceeded(&self) -> bool {
        // Note: need at least Acquire constraint in order for store to be visible across threads
        self.overflow.load(Ordering::Acquire)
    }

    pub fn reset(&self) {
        StackGuard::reset();
    }
}

/// Initializes the global state, more precisely modifes panic hook such that:
/// - panics raised from StackLimit::panic_if_exceeded() are silenced;
/// - other panics are passed through the original panic hook.
pub fn init() {
    use std::sync::Once;
    static INIT: Once = Once::new();
    INIT.call_once(|| {
        let original_hook = std::panic::take_hook();
        std::panic::set_hook(Box::new(move |panic_info| {
            let mut swallow = false;
            if let Some(location) = panic_info.location() {
                if location.file() == file!() {
                    // if panic comes from this file, it must be due to StackOverflow
                    swallow = true;
                }
            }
            if !swallow {
                original_hook(&panic_info);
            }
        }));
    });
}

// Implementation details (hidden to facilitate swapping in something less hacky)
mod detail {
    use std::cell::RefCell;

    thread_local!(static STK_GUARD: RefCell<StackGuard> = RefCell::new(StackGuard::new()));

    pub struct StackGuard {
        min: usize,
        max: usize,
    }

    impl StackGuard {
        pub fn new() -> Self {
            let mut ret = StackGuard {
                min: std::usize::MAX,
                max: std::usize::MIN,
            };
            ret.update();
            ret
        }

        pub fn reset() {
            STK_GUARD.with(|stk| stk.replace(Self::new()));
        }

        pub fn exceeds_size(bytes: usize) -> bool {
            STK_GUARD.with(|stk| stk.borrow_mut().update() > bytes)
        }

        #[inline(never)] // ensure that local variable gets address on stack
        fn update(&mut self) -> usize {
            let local = 1;
            let cur = &local as *const _ as usize;
            self.min = std::cmp::min(self.min, cur);
            self.max = std::cmp::max(self.max, cur);
            self.size()
        }

        fn size(&self) -> usize {
            if self.max >= self.min {
                self.max - self.min
            } else {
                self.min - self.max
            }
        }
    }
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;

    pub(crate) struct StackBounded<'a> {
        pub(crate) limit: &'a StackLimit,
    }

    impl StackBounded<'_> {
        fn ackermann(&self, m: i64, n: i64) -> Result<i64, ()> {
            if self.limit.check_exceeded() {
                return Err(());
            }

            if m == 0 {
                Ok(n + 1)
            } else if n == 0 {
                self.ackermann(m - 1, 1)
            } else {
                let inner: i64 = self.ackermann(m, n - 1)?;
                self.ackermann(m - 1, inner)
            }
        }

        pub(crate) fn min_n_that_fails_ackermann(&self, m: i64) -> i64 {
            for n in 2..20 {
                if self.ackermann(m, n).is_err() {
                    return n;
                }
            }
            0
        }
    }

    #[test]
    fn ackermann_m_eq_3_growing_stack_avoids_overflow() {
        std::thread::spawn(|| {
            // new thread to avoid relying on StackGuard thread-localness
            const M: i64 = 3;
            const LIMIT: usize = 25_000; // 25 KB
            let limit = StackLimit::relative(LIMIT);
            let bounded = StackBounded { limit: &limit };

            let n_err = bounded.min_n_that_fails_ackermann(M);

            assert!(bounded.ackermann(M, n_err).is_err());

            // Ackermann recursion depth grows ~2x when n is increased by 1 and m is fixed,
            // so triple stack size (to avoid rounding errors) should always be sufficient
            let limit = StackLimit::relative(3 * LIMIT);
            let bounded = StackBounded { limit: &limit };
            assert!(bounded.ackermann(M, n_err).is_ok());
        })
        .join()
        .unwrap();
    }

    #[test]
    fn units() {
        assert_eq!(1_024, KI);
        assert_eq!(1_048_576, MI);
        assert_eq!(1_073_741_824, GI);
    }
}
