// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{StackLimit, GI, KI, MI};
use thiserror::Error;

use std::fmt;

/// Determining the current thread size cannot be done portably,
/// therefore assume the worst (running on non-main thread with min size, 2MiB)
const DEFAULT_CURRENT_STACK_MAX: usize = 2 * MI;
const DEFAULT_NONMAIN_STACK_MAX: usize = 1 * GI;

/// Parameters for executing retryable functions on elastic stack;
/// optional ones have defaults.
#[derive(Default, Debug)]
pub struct Job {
    pub current_stack_max: Option<usize>,
    pub nonmain_stack_min: usize,
    pub nonmain_stack_max: Option<usize>,
}

impl Job {
    /// Repeatedly runs a retryable computation produced by `make_retryable`,
    /// guarding each attempt by _exponentially_ increasing stack limits
    /// in order to keep the asymoptic time complexity the same; it assumes that
    /// retryable calls `StackLimit::panic_if_exceeded` before it's too late.
    /// Since the stack size it being doubled, the time is: t + 2*t + 4*t + ...
    /// where the total time with unbounded stack is T=k*t; therefore it is
    /// bounded by 2*T (much less in practice due to superlinear parsing time).
    /// The retryable function has exactly the requirements of both:
    /// - std::panic::catch_unwind
    /// - std::thread::spawn
    /// so the `make_retryable` needs to avoid imposing other restrictions.
    /// The on_retry function will be called after each unsuccessful attempt
    /// with the enforced stack limit, which is equal to `self.current_stack_limit`
    /// and corresponds to the current thread in the first attempt, or a number number
    /// between `self.nonmain_stack_min` and `self.nonmain_stack_max`.  It is `FnMut`
    /// so that the caller can easily report progress (stateful in general).
    ///
    /// `compute_stack_slack` is a function which takes actual stack size and
    /// returns a "slack" stack size. slack stack must be
    /// larger than stack increase between two consecutive calls of `check_exceeded`.
    /// For example, `compute_stack_slack` should return a value which is greater than
    /// `std::mem_size_of<A_0>()` + ... + `std::mem_size_of<A_n>()`.
    /// ```
    /// fn foo(sl: &StackLimit) -> {
    ///     sl.check_exceeded();
    ///     let _: A_0 = ...;
    ///     ...
    ///     let _: A_n = ...;
    /// }
    /// ```
    pub fn with_elastic_stack<F, T>(
        &self,
        make_retryable: impl Fn() -> F,
        on_retry: &mut impl FnMut(usize),
        compute_stack_slack: StackSlackFunction,
    ) -> Result<T, JobFailed>
    where
        F: FnOnce(&StackLimit, NonMainStackSize) -> T,
        F: Send + 'static + std::panic::UnwindSafe,
        T: Send + 'static,
    {
        // Eagerly validate job parameters via getters, so we can panic before executing
        let max_stack_size = self.check_nonmain_space_max();
        let min_nonmain_size = self.check_nonmain_space_min();

        super::init(); // very important (enables panic recovery)!

        let mut stack_size = self.current_stack_max.unwrap_or(DEFAULT_CURRENT_STACK_MAX);
        let mut nonmain_stack_size = None; // 1st attempt is made in the current thread
        loop {
            let relative_stack_size = stack_size - compute_stack_slack(stack_size);
            if relative_stack_size >= stack_size {
                // check for underflow (i.e., if stack_slack > stack_size)
                panic!("bad compute_stack_slack (must return < stack_size)");
            }

            let retryable = make_retryable();
            let try_retryable = move || {
                let stack_limit = StackLimit::relative(relative_stack_size);
                stack_limit.reset();
                let stack_limit_ref = &stack_limit;
                match std::panic::catch_unwind(move || {
                    retryable(stack_limit_ref, nonmain_stack_size)
                }) {
                    Ok(result) => Some(result),
                    Err(_) if stack_limit.exceeded() => None,
                    Err(msg) => std::panic::panic_any(msg),
                }
            };

            // Call retryable on the current thread in the 1st iteration or nonmain thread otherwise.
            let result_opt = match nonmain_stack_size {
                None => try_retryable(),
                Some(stack_size) => std::thread::Builder::new()
                    .stack_size(stack_size)
                    .spawn(try_retryable)
                    .expect("ERROR: thread::spawn")
                    .join()
                    .expect("ERROR: failed to wait on new thread"),
            };
            if let Some(result) = result_opt {
                return Ok(result);
            } else {
                on_retry(stack_size)
            }

            // Avoid eagerly wasting of space that will not be used in practice (WWW),
            // but only for degenerate test cases (/test/{slow,quick}), by starting off
            // with small stack (default thread) then fall back to bigger ones (nonmain thread).
            let next_stack_size = if nonmain_stack_size.is_none() {
                min_nonmain_size
            } else {
                // exponential backoff to limit parsing time to at most twice as long
                2 * stack_size
            };
            if next_stack_size > max_stack_size {
                return Err(JobFailed {
                    max_stack_size_tried: stack_size,
                });
            }
            nonmain_stack_size = Some(next_stack_size);
            stack_size = next_stack_size;
        }
    }

    // TODO(hrust) a Builder pattern that does validation at the end would be cleaner

    fn check_nonmain_space_max(&self) -> usize {
        self.nonmain_stack_max.unwrap_or(DEFAULT_NONMAIN_STACK_MAX)
    }
    fn check_nonmain_space_min(&self) -> usize {
        if self.nonmain_stack_min > self.check_nonmain_space_max() {
            panic!("min > max for nonmain stack space");
        }
        self.nonmain_stack_min
    }
}

/// Some(stack_size) if it is definitely a non-main thread, None otherwise.
pub type NonMainStackSize = Option<usize>;

// Computes the amount of bytes that must remain before execution is aborted
// because it would have really hit the stack limit; i.e., cushion space.
pub type StackSlackFunction = fn(usize) -> usize;

/// Computes stack slack the same way as HHVM does it in
/// https://github.com/facebook/hhvm/blob/master/hphp/runtime/base/request-info.h
pub fn hhvm_stack_slack(_stack_size: usize) -> usize {
    MI
}

#[derive(Error, Debug)]
pub struct JobFailed {
    pub max_stack_size_tried: usize,
}
impl fmt::Display for JobFailed {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "{}::JobFailed: retry job would exceed maximum nonmain stack of {} KiB",
            module_path!(),
            self.max_stack_size_tried / KI,
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::StackLimit;

    #[test]
    fn with_elastic_stack_correctly_grows_stack_after_retry() {
        const SLACK_SPACE: usize = 500;
        fn get_slack_space(_: usize) -> usize {
            SLACK_SPACE
        }

        let job = super::Job {
            current_stack_max: Some(250 + SLACK_SPACE),
            nonmain_stack_min: 1000,
            nonmain_stack_max: Some(10_000),
            ..Default::default()
        };

        let make_expo_grower = || {
            Box::new(|limit: &StackLimit, _: super::NonMainStackSize| {
                eprintln!("limit = {} B", limit.get());
                let bounded = crate::tests::StackBounded {
                    // Note: safe because we're not leaking bounded from the closure
                    limit: unsafe { std::mem::transmute(limit) },
                };
                bounded.min_n_that_fails_ackermann(3);
                limit.panic_if_exceeded();
            })
        };

        let mut stack_sizes = Vec::<usize>::new();
        let mut on_retry = |stack_size| {
            stack_sizes.push(stack_size);
        };

        assert!(
            job.with_elastic_stack(&make_expo_grower, &mut on_retry, get_slack_space,)
                .is_err()
        );

        assert_eq!(
            stack_sizes,
            vec![job.current_stack_max.unwrap(), 1000, 2000, 4000, 8000]
        );
    }
}
