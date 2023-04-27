// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Configuration for the balanced partitioning algorithm.
#[derive(Debug)]
pub struct BalanceConfig {
    /// Maximum recursion depth.
    pub max_depth: u32,

    /// Maximum number of optimization rounds at each step.
    pub max_rounds: u32,

    /// Smallest row worth optimizing.
    pub min_row_degree: u32,

    /// Largest row worth optimizing (inclusive).
    pub max_row_degree: u32,

    /// Stop recursing once we get to this many docs. Must be >= 2.
    pub min_num_docs: usize,

    /// How many of the top levels of recursion should we run in
    /// "parallel mode" before switching to "serial mode"?
    ///
    /// We have both serial and parallel algorithms for some
    /// steps. The serial ones are faster if there's only one thread
    /// (e.g. don't use atomic ops, avoid rayon overhead), so we only
    /// want to use parallel algos up to the point where all of our
    /// threads have something useful to do. For example, at the top
    /// level, before we have split at all, it's best for threads to
    /// all help out there. But after enough bisections we know
    /// there's enough work for all threads, so we don't need to use
    /// parallelism within a step.
    ///
    /// The default value for this attempts to maximize the amount of
    /// work done in serial mode while keeping all threads busy, which
    /// is about twice as fast as doing everything in parallel modes.
    ///
    /// Even after going beyond parallel depth, each left+right recursion
    /// always makes the right recursion available for Rayon work stealing,
    /// so we don't give up on high-level parallelism, just on parallel
    /// algorithms within a single step.
    pub max_par_depth: u32,

    /// A number [0.0, 1.0] indicating when we should stop early
    /// swapping docs. When <= this fraction of docs get swapped, we stop
    /// and proceed recursing down the left and right sides.
    pub quiesced_fraction: f64,

    /// When each step of the recursion finishes, we run a greedy algorithm on a
    /// sliding window for whatever remains. This is the size of that window.
    /// Larger windows will optimize better, but run slower.
    pub max_leaf_window_size: usize,
}

impl Default for BalanceConfig {
    fn default() -> Self {
        // By default, run the top few levels of the recursion in
        // parallel, so idle threads can help out. But once we've
        // recursed enough, switch to running many single-threaded
        // algorithms in parallel for different parts of the recursion
        // tree, as those algorithms run faster than equivalent
        // parallel algorithms (using atomics etc.) run on one thread.
        //
        // Note that each time we recurse down the tree we can always
        // spawn off new subtrees in parallel, this only affects
        // parallelism within a single tree node.
        let max_par_depth = rayon::current_num_threads()
            .next_power_of_two()
            .trailing_zeros();

        Self {
            max_depth: u32::MAX,
            max_rounds: 20,
            min_row_degree: 2,
            max_row_degree: u32::MAX,
            min_num_docs: 64,
            max_par_depth,

            quiesced_fraction: 0.02,

            max_leaf_window_size: 64,
        }
    }
}
