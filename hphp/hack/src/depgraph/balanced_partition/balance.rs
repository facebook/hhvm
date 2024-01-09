// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Reverse;
use std::sync::atomic::AtomicBool;
use std::sync::atomic::AtomicU32;
use std::sync::atomic::Ordering;

use rayon::prelude::*;

use crate::config::BalanceConfig;
use crate::timers::TimerGuard;
use crate::timers::Timers;

/// This is an opaque identifier for a Doc supplied by the user of this crate to
/// identify which Doc is which after `optimize_doc_order` permutes them.
#[derive(Default)]
pub struct ExternalId(pub u32);

pub struct Doc {
    /// This is a sorted list of what we're calling row numbers. You can think of them as
    /// representing a set of indexed terms in a Doc, or alternatively as in-edges in a
    /// directed graph.
    ///
    /// Our ultimate goal is to compress a set of Docs indexed by these row numbers.  That
    /// data structure would look like an array of Doc numbers for each row (indicating all
    /// Docs that contain that row number). If we permute Doc numbers so that the numbers in
    /// a row are clustered together, delta coding will make the representation of that row
    /// smaller.
    ///
    /// As an optimization, the exact meaning of row numbers changes throughout the
    /// algorithm and ultimately this vec gets clobbered. As we recursively partition the
    /// optimization space, we'll renumber the edge_lists for Docs in that partition to keep
    /// the row numbering dense. This improves cache locality and reduces memory
    /// utilization.  For example, if we originally had 1000 different values across all
    /// 10000 Docs, this would contain some number of values in the range [0, 1000). As we
    /// recurse and (say) get down to 2500 docs, we may find ourselves with only 500 rows
    /// mentioned by any of those Docs. So we renumber the edge_lists in those docs to only
    /// contain [0, 500).
    pub edge_list: Vec<u32>,

    /// How many identical docs this represents.
    pub weight: u32,

    /// Unique identifier for this doc, so the caller can recognize it.
    pub id: ExternalId,
}

/// A lookup table used to calculate `N * log2(N + 1)` quickly, where N as integer.
///
/// The results it produces are not the floating-point answer, but rather that answer
/// scaled up then rounded to an integer.
struct CostTable {
    /// Holds N * log2(N + 1), but normalized so the largest table entry equals u32::MAX.
    ///
    /// The intent here is to let us use integer arithmetic everywhere, as it is associative
    /// and can be used with atomics if we want.
    table: Vec<u32>,
}

impl CostTable {
    /// Create a `CostTable` whose `cost` method accepts arguments up to and including `max_arg`.
    fn new(max_arg: usize) -> Self {
        // Compute the answer for `n` as an f64, which we'll normalize to a `u32`.
        fn f64_value(n: usize) -> f64 {
            let n = n as f64;
            n * (n + 1.0).log2()
        }

        // Compute a scaling factor that lets us normalize everything to u32::MAX.
        let max_value = f64_value(max_arg);
        let scale = if max_value == 0.0 {
            0.0
        } else {
            u32::MAX as f64 / max_value
        };

        let table: Vec<u32> = (0..=max_arg)
            .into_par_iter()
            .map(move |n| {
                let f = f64_value(n) * scale;
                let neg_cost_u64: u64 = f.round() as u64;

                // Use try_into just in case rounding pushes us over the edge.
                neg_cost_u64.try_into().unwrap_or(u32::MAX)
            })
            .collect();

        Self { table }
    }

    /// Returns the cost (an integer proportional to `-(N * log2(N + 1)))`.
    fn lookup(&self, n: u32) -> i64 {
        -(self.table[n as usize] as i64)
    }

    /// An estimate for the encoding cost for a sequence of sorted integers,
    /// where `deg_a` values are on the left half, and `deg_b` are on the
    /// right half.
    ///
    /// The idea here is that placing more values on one half or the other is
    /// cheaper to encode, because the average delta between values get smaller,
    /// meaning fewer bits are needed to encode it.
    ///
    /// Stepping up a level, minimizing costs corresponds to numbering edge lists
    /// densely in the original, untransposed graph.
    fn cost(&self, deg_a: u32, deg_b: u32) -> i64 {
        self.lookup(deg_a) + self.lookup(deg_b)
    }
}

/// Similar to `vec![Default::default(); len]`, but for when `T` does not support `clone()`.
fn vec_with_default<T: Default>(len: usize) -> Vec<T> {
    let mut v = Vec::with_capacity(len);
    v.resize_with(len, Default::default);
    v
}

fn compute_row_move_gains(
    partition: &mut Partition,
    cost_table: &CostTable,
) -> (i64, Vec<i64>, Vec<i64>) {
    let mut total_cur_cost = 0i64;

    // We could do this in parallel, but we don't spend enough time doing it to
    // make it worthwhile.
    let (l_to_r_gains, r_to_l_gains): (Vec<i64>, Vec<i64>) = partition
        .total_degrees
        .iter_mut()
        .zip(partition.right_degrees.iter_mut())
        .map(|(t, r)| {
            let total_deg = *t.get_mut();
            let right_deg = *r.get_mut();
            let left_deg = total_deg - right_deg;

            let cur_cost = cost_table.cost(left_deg, right_deg);
            total_cur_cost += cur_cost;

            let cost_after_move_l_to_r = cost_table.cost(left_deg.saturating_sub(1), right_deg + 1);
            let l_to_r_move_gain = cur_cost - cost_after_move_l_to_r;

            let cost_after_move_r_to_l = cost_table.cost(left_deg + 1, right_deg.saturating_sub(1));
            let r_to_l_move_gain = cur_cost - cost_after_move_r_to_l;

            (l_to_r_move_gain, r_to_l_move_gain)
        })
        .unzip();

    (total_cur_cost, l_to_r_gains, r_to_l_gains)
}

fn compute_doc_move_gains<'a>(
    docs: &'a mut [Doc],
    l_to_r_gains: Vec<i64>,
    r_to_l_gains: Vec<i64>,
    left_partition_len: usize,
    is_parallel: bool,
) -> Vec<(i64, &mut Doc)> {
    let sum_gains = move |(i, doc): (usize, &'a mut Doc)| {
        let mut gain = 0;

        let gains = if i < left_partition_len {
            &l_to_r_gains[..]
        } else {
            &r_to_l_gains[..]
        };

        for &row in doc.edge_list.iter() {
            gain += gains[row as usize];
        }

        (gain, doc)
    };

    if is_parallel {
        docs.par_iter_mut().enumerate().map(sum_gains).collect()
    } else {
        docs.iter_mut().enumerate().map(sum_gains).collect()
    }
}

/// This approximately sorts `left_gains` and `right_gains` by their i64
/// cost field, putting the largest values first. But not quite, for speed.
///
/// If we didn't care about speed it would be completely correct to use
/// this implementation:
///
/// ```
/// left_gains.sort_unstable_by_key(|a| Reverse(a.0));
/// right_gains.sort_unstable_by_key(|a| Reverse(a.0));
/// ```
///
/// But we can do better, because we know the caller doesn't need a full sort.
/// The caller iterates through values in both arrays pairwise until the sum
/// is nonpositive, then stops. Any values it visits before stopping it swaps
/// to the other side of the partition, and the order in which it does this
/// does not matter.
///
/// Suppose we did fully sort both arrays. We would end up with three regions
/// (P == Positive, N = Nonpositive). We'll assume without loss of generality that
/// right has fewer positive values:
///
///    left     right
///    ----     -----
///     P        P     |
///     ...      ...   |---  Positive/Positive
///     P        P     |
///
///     P        N     |
///     ...      ...   |---  Positive/Negative (this size is called `overlap` below)
///     P        N     |
///
///     N        N     |
///     ...      ...   |---  Negative/Negative
///     N        N     |
///
/// We only need a proper sort for the P/N section in the middle. Everything else
/// can just be roughly bucketed.
///
/// - In the P/P section, we're going to swap everything in both arrays across the partition
///   because their sums are positive. It's not important exactly which pairs get switched,
///   we only care that everything on left moves to right and vice versa.
/// - Similarly, we know in the N/N section we're going to swap nothing, so we don't care
///   about the exact ordering within either N section.
/// - For P/N, we fully sort both sides (technically even this isn't required, but that
///   starts to get complicated).
fn partially_sort_move_gains<'a, 'b: 'a>(
    left_gains: &'a mut [(i64, &'b mut Doc)],
    right_gains: &'a mut [(i64, &'b mut Doc)],
) {
    let left_num_pos = itertools::partition(left_gains.iter_mut(), |g| g.0 > 0);
    let right_num_pos = itertools::partition(right_gains.iter_mut(), |g| g.0 > 0);

    if left_num_pos == right_num_pos {
        // There is no P/N section, we are done.
        return;
    }

    // Figure out which side has more P values, and call its positive values `pos`.
    // Call the other side's negative values `neg`.
    //
    // These correspond to the left column P values and right column N values in
    // the top-of-function comment.
    let (mut neg, mut pos, overlap) = if left_num_pos > right_num_pos {
        (
            &mut right_gains[right_num_pos..],
            &mut left_gains[..left_num_pos],
            left_num_pos - right_num_pos,
        )
    } else {
        (
            &mut left_gains[left_num_pos..],
            &mut right_gains[..right_num_pos],
            right_num_pos - left_num_pos,
        )
    };

    // Move the smallest P values into the overlap section. We could legally put the largest ones
    // there, but empirically that seems to do a bit worse.
    if overlap < pos.len() {
        let (_, _, smallest) =
            pos.select_nth_unstable_by_key(pos.len() - overlap, |a| Reverse(a.0));
        pos = smallest;
    }

    // Sort to place the largest P values in the overlap section first.
    pos.sort_unstable_by_key(|a| Reverse(a.0));

    // Move the largest (that is, closest to 0.0) N values into the overlap section.
    if overlap < neg.len() {
        // Grab just the largest N values for the overlap section.
        let (largest, _, _) = neg.select_nth_unstable_by_key(overlap - 1, |a| Reverse(a.0));
        neg = largest;
    }

    // Sort to place the largest N values in the overlap section first.
    neg.sort_unstable_by_key(|a| Reverse(a.0));
}

/// Update counters for the docs that moved.
fn par_update_partition_for_swaps(swaps: Vec<(&mut Doc, &mut Doc)>, partitions: &Partition) {
    let right_degrees = &partitions.right_degrees[..];

    swaps
        .into_par_iter()
        .for_each(move |(r_to_l_doc, l_to_r_doc)| {
            let sub_weight = r_to_l_doc.weight;
            for &edge in r_to_l_doc.edge_list.iter() {
                right_degrees[edge as usize].fetch_sub(sub_weight, Ordering::Relaxed);
            }

            let add_weight = l_to_r_doc.weight;
            for &edge in l_to_r_doc.edge_list.iter() {
                right_degrees[edge as usize].fetch_add(add_weight, Ordering::Relaxed);
            }
        });
}

/// Serial version of par_update_partition_for_swaps. Avoids atomics.
fn ser_update_partition_for_swaps(swaps: Vec<(&mut Doc, &mut Doc)>, partitions: &mut Partition) {
    let right_degrees = &mut partitions.right_degrees[..];

    for (r_to_l_doc, l_to_r_doc) in swaps {
        let sub_weight = r_to_l_doc.weight;
        for &edge in r_to_l_doc.edge_list.iter() {
            let n = right_degrees[edge as usize].get_mut();
            *n = n.wrapping_sub(sub_weight);
        }

        let add_weight = l_to_r_doc.weight;
        for &edge in l_to_r_doc.edge_list.iter() {
            let n = right_degrees[edge as usize].get_mut();
            *n = n.wrapping_add(add_weight);
        }
    }
}

/// Remove any trailing empty zeros from the Vec.
fn remove_trailing_zeros(v: &mut Vec<AtomicU32>) {
    let last_nonzero = v.iter_mut().rposition(|n| *n.get_mut() != 0);
    v.truncate(last_nonzero.map_or(0, |n| n + 1));
}

/// This semantically holds the total weights on the left and right halves of
/// the partition, which we need for our cost function.
///
/// For efficiency, we physically represent it as `(total, right)` rather
/// than `(left, right)`, with `left` implicitly being `total - right`.
/// This is so hot operations can just increment or decrement `right`, rather
/// than both a `left` and `right` vec.
#[derive(Default)]
struct Partition {
    /// For each row, the sum of Doc weights across that row for Docs that have that row in
    /// their `edge_list`.
    total_degrees: Vec<AtomicU32>,

    /// Like `total_degrees`, but only sums Docs on the right half of the partition.
    ///
    /// The degrees for the left half can of course be computed as `total - right`.
    right_degrees: Vec<AtomicU32>,
}

/// Compute the sum of weights across each row.
fn compute_degrees(docs: &[Doc], num_rows: usize, is_parallel: bool) -> Vec<AtomicU32> {
    let mut degrees: Vec<AtomicU32> = vec_with_default(num_rows);

    if is_parallel {
        docs.par_iter().for_each(|doc| {
            let weight = doc.weight;
            for &row in doc.edge_list.iter() {
                degrees[row as usize].fetch_add(weight, Ordering::Relaxed);
            }
        });
    } else {
        for doc in docs.iter() {
            let weight = doc.weight;
            for &row in doc.edge_list.iter() {
                *degrees[row as usize].get_mut() += weight;
            }
        }
    }

    degrees
}

/// Simplify the problem as we recurse by deleting rows.
fn remove_useless_rows<'a>(
    total_degrees: &mut Vec<AtomicU32>,
    docs: &'a mut [Doc],
    config: &BalanceConfig,
    is_parallel: bool,
) -> &'a mut [Doc] {
    // TODO: Only bother doing anything if we find X% of rows are useless?
    // The full rows are of course costliest, they could be weighted more.

    let num_docs = docs.len() as u32;
    if num_docs == 0 {
        return docs;
    }

    // A row with too few docs isn't worth optimizing. Similarly, a row with every
    // possible docs in it isn't interesting as permuting never has any effect.
    //
    // TODO: Every doc except one is equivalently useless to one doc, so
    // treat them the same?
    let min_deg = config.min_row_degree;
    let max_deg = std::cmp::min(num_docs - 1, config.max_row_degree);

    let is_useless = move |total_deg: u32| total_deg < min_deg || total_deg > max_deg;

    // Skip ahead to the first useless row, hoping we find none and can return early.
    let first_discard_index = match total_degrees
        .iter_mut()
        .map(|n| *n.get_mut())
        .position(is_useless)
    {
        Some(i) => i,
        None => return docs,
    };

    // Because we're removing at least one row, we'll need to renumber the remaining rows
    // to keep the numbering dense.
    const NO_REMAP: u32 = u32::MAX;
    let mut remap = vec![NO_REMAP; total_degrees.len()];

    for (i, r) in remap.iter_mut().take(first_discard_index).enumerate() {
        *r = i as u32;
    }

    let mut out = first_discard_index;

    for (i, remap_ref) in remap.iter_mut().enumerate().skip(first_discard_index + 1) {
        if !is_useless(*total_degrees[i].get_mut()) {
            // Keep this row, but remap it later.
            let n = *total_degrees[i].get_mut();
            *total_degrees[out].get_mut() = n;
            *remap_ref = out as u32;
            out += 1;
        }
    }

    total_degrees.truncate(out);

    // Renumber and shrink docs. If we ever see an empty doc, we'll have extra cleanup to do.
    let found_empty_doc = if is_parallel {
        let found_empty_doc = AtomicBool::new(false);

        docs.par_iter_mut().for_each(|doc| {
            if doc
                .edge_list
                .last()
                .map_or(false, |&n| (n as usize) >= first_discard_index)
            {
                doc.edge_list.retain_mut(|n| {
                    let r = remap[*n as usize];
                    *n = r;
                    r != NO_REMAP
                });
            }

            if doc.edge_list.is_empty() {
                found_empty_doc.store(true, Ordering::Relaxed);
            }
        });

        found_empty_doc.into_inner()
    } else {
        let mut found_empty_doc = false;

        for doc in docs.iter_mut() {
            if doc
                .edge_list
                .last()
                .map_or(false, |&n| (n as usize) >= first_discard_index)
            {
                doc.edge_list.retain_mut(|n| {
                    let r = remap[*n as usize];
                    *n = r;
                    r != NO_REMAP
                });
            }

            if doc.edge_list.is_empty() {
                found_empty_doc = true;
            }
        }

        found_empty_doc
    };

    if found_empty_doc {
        // We found some empty docs, move them to the end and forget about them.
        let split = itertools::partition(docs.iter_mut(), |doc| !doc.edge_list.is_empty());
        &mut docs[..split]
    } else {
        docs
    }
}

/// Permute a slice of Docs using a greedy sliding window algorithm rather than
/// recursive bisection. We use this at the leaves of our recursion to try to maximize
/// the effectiveness of RLE.
fn optimize_base_case(docs: &mut [Doc], num_rows: usize, config: &BalanceConfig, timers: &Timers) {
    if docs.len() <= 2 {
        return;
    }

    let _timer_guard = TimerGuard::new(&timers.leaf_nanos);

    // This is u32::MAX - 1 so that we can do `prev_index + 1` below without wraparound.
    const NOT_YET_SEEN: u32 = u32::MAX - 1;

    let mut latest: Vec<(u32, bool)> = vec![(NOT_YET_SEEN, false); num_rows];

    // This is a simple greedy algorithm that just finds the next Doc in a sliding window that seems
    // to be cheapest to emit, emits it, and moves on to the next. The intent is to place very similar
    // Docs next to each other, or nearby, even if recursively partitioning didn't do it (e.g. because
    // it didn't recurse all the way down).
    for i in 0..docs.len() - 1 {
        let mut best_index = i;
        let mut best_score = i64::MIN;

        for (j, doc) in docs
            .iter()
            .enumerate()
            .skip(i)
            .take(config.max_leaf_window_size)
        {
            let mut score = 0i64;

            // This implementation is completely unprincipled and has little empirical basis,
            // it's just some intuitive rough guess for a cost function that might help.
            for &row in doc.edge_list.iter() {
                let (prev_index, prev_is_rle) = latest[row as usize];
                if prev_index == NOT_YET_SEEN {
                    // We've never seen this before (no recent repeats), so penalize it a bit.
                    score -= docs.len() as i64;
                } else {
                    let distance = i as i64 - prev_index as i64;

                    if distance == 1 {
                        if prev_is_rle {
                            // Continuing an RLE block is ideal.
                            score += 10_000_000;
                        } else {
                            // Starting a new RLE block is good too.
                            score += 9_000_000;
                        }
                    } else {
                        // No RLE, but bias toward repeating recent occurrences
                        // since that will yield smaller deltas.
                        // Really this should involve a log2, but meh.
                        score -= distance;
                    }
                }
            }

            if score > best_score {
                best_score = score;
                best_index = j;
            }
        }

        docs.swap(i, best_index);

        // We've made our choice, update `latest` to indicate what's been emitted.

        let newly_placed_index = i as u32;

        // Go ahead and grab the edge_list here, so we'll free it when we iterate
        // through it below. No one should be looking at this Vec any more, we've
        // been modifying its contents as we've recursed, and the contents aren't
        // needed any more.
        let edges = std::mem::take(&mut docs[i].edge_list);

        for edge in edges {
            // Read the old state.
            let l = &mut latest[edge as usize];
            let (prev_index, _was_rle) = *l;

            // Write the new state.
            let is_now_rle = prev_index + 1 == newly_placed_index;
            *l = (newly_placed_index, is_now_rle);
        }
    }
}

/// This is the heart of the balanced partitioning algorithm; see
/// https://www.kdd.org/kdd2016/papers/files/rpp0883-dhulipalaAemb.pdf for
/// an explanation.
///
/// This permutes `docs` to try to make the left and right halves more
/// self-similar, so they compress better, then recurses over the left
/// and right halves.
///
/// For speed, rather than actually recursing we loop down the left side
/// and post a task for the right side to a `rayon::Scope`, so an idle
/// thread can steal that work if it wants.
fn recursively_balance<'a>(
    scope: &rayon::Scope<'a>,
    mut docs: &'a mut [Doc],
    cost_table: &'a CostTable,
    mut total_degrees: Vec<AtomicU32>,
    mut depth: u32,
    config: &'a BalanceConfig,
    timers_per_depth: &'a [Timers],
    mut num_log_tokens: usize,
) {
    // This will loop to recurse down the left hand side.
    loop {
        let timers = &timers_per_depth[std::cmp::min(depth as usize, timers_per_depth.len() - 1)];

        // Use parallel algos within this depth? See BalanceConfig::max_par_depth docs.
        let is_parallel = depth < config.max_par_depth;

        let remove_useless_timer = TimerGuard::new(&timers.remove_useless_nanos);

        docs = remove_useless_rows(&mut total_degrees, docs, config, is_parallel);

        let setup_timer = TimerGuard::handoff(remove_useless_timer, &timers.setup_nanos);

        let left_partition_len = docs.len() / 2;
        let right_partition_len = docs.len() - left_partition_len;

        remove_trailing_zeros(&mut total_degrees);
        let right_degrees = compute_degrees(
            &docs[left_partition_len..],
            total_degrees.len(),
            is_parallel,
        );

        drop(setup_timer);

        let mut partitions = Partition {
            total_degrees,
            right_degrees,
        };

        if num_log_tokens != 0 {
            log::info!(
                "Starting depth {}, {} rows, {} docs",
                depth,
                partitions.total_degrees.len(),
                docs.len()
            );
        }

        // We use this to notice when the cost stops improving.
        let mut prev_cost = i64::MAX;

        let mut swaps_capacity_guess = left_partition_len / 2;

        // Repeatedly try to improve the left/right partitioning by swapping Docs across the boundary.
        for round_number in 0..config.max_rounds {
            if num_log_tokens != 0 {
                log::info!("Starting depth {} round {}", depth, round_number);
            }

            let row_move_gains_timer = TimerGuard::new(&timers.row_move_gains_nanos);

            let (cur_cost, l_to_r_gains, r_to_l_gains) =
                compute_row_move_gains(&mut partitions, cost_table);
            if cur_cost >= prev_cost {
                // We didn't improve since last time, so just stop looking.
                break;
            }
            prev_cost = cur_cost;

            let doc_move_gains_timer =
                TimerGuard::handoff(row_move_gains_timer, &timers.doc_move_gains_nanos);

            let mut doc_move_gains = compute_doc_move_gains(
                docs,
                l_to_r_gains,
                r_to_l_gains,
                left_partition_len,
                is_parallel,
            );

            // Place the largest values first.
            let sort_timer = TimerGuard::handoff(doc_move_gains_timer, &timers.sort_nanos);
            let (left_gains, right_gains) = doc_move_gains.split_at_mut(left_partition_len);
            partially_sort_move_gains(left_gains, right_gains);

            // Swap values across the partition as long as it seems net-profitable to do so.
            // We'll start with the value that most wants to swap left to right, and the one
            // that most wants to swap right to left. If the sum of the gains of this swap
            // is positive, we do it. Then we move on to the second-most eager docs, etc.
            let _swap_timer = TimerGuard::handoff(sort_timer, &timers.swap_nanos);

            // Scratch space for our swaps array.
            let mut swaps: Vec<(&mut Doc, &mut Doc)> = Vec::with_capacity(swaps_capacity_guess);

            for ((left_gain, left_doc), (right_gain, right_doc)) in
                left_gains.iter_mut().zip(right_gains)
            {
                if *left_gain + *right_gain <= 0 {
                    break;
                }

                let left_doc: &mut Doc = left_doc;
                let right_doc: &mut Doc = right_doc;
                std::mem::swap(left_doc, right_doc);

                // Remember to actually swap them below.
                swaps.push((left_doc, right_doc));
            }

            swaps_capacity_guess = swaps.len();
            let num_docs_swapped = swaps.len() * 2;

            if num_log_tokens != 0 {
                log::info!(
                    "Depth {}, round {}, moved {} values",
                    depth,
                    round_number,
                    num_docs_swapped
                );
            }

            if num_docs_swapped != 0 {
                if is_parallel {
                    par_update_partition_for_swaps(swaps, &partitions);
                } else {
                    ser_update_partition_for_swaps(swaps, &mut partitions);
                }
            }

            if num_docs_swapped as f64 <= docs.len() as f64 * config.quiesced_fraction {
                // If we didn't swap very much this time, just stop.
                break;
            }
        }

        let mut right_total_degrees = partitions.right_degrees;
        let num_rows = right_total_degrees.len();

        depth += 1;

        if depth > config.max_depth
            || std::cmp::min(left_partition_len, right_partition_len) < config.min_num_docs
        {
            // Base case: if we're not recursing both ways, just stop.
            //
            // Someday, if we support imbalanced partitions, we might want to loop down
            // just one side or the other.
            optimize_base_case(docs, num_rows, config, timers);
            break;
        }

        // Prepare to recurse down the left and right halves.
        //
        // We will manually tail-call down the left half by looping.
        let (left_docs, right_docs) = docs.split_at_mut(left_partition_len);

        // Create `left_total_degrees`. It starts as the total degrees from the previous
        // recursion depth, so we need to subtract off the right degrees to get the totals
        // just for the left side.
        let mut left_total_degrees = partitions.total_degrees;
        for (l, r) in left_total_degrees
            .iter_mut()
            .zip(right_total_degrees.iter_mut())
        {
            *l.get_mut() -= *r.get_mut();
        }

        // Divvy up the permission-to-log token so we log a limited amount overall.
        let orig_num_tokens = num_log_tokens;
        num_log_tokens = (num_log_tokens + 1) / 2;
        let right_num_log_tokens = orig_num_tokens - num_log_tokens;

        // Defer the right-side work for later.
        scope.spawn(move |scope| {
            recursively_balance(
                scope,
                right_docs,
                cost_table,
                right_total_degrees,
                depth,
                config,
                timers_per_depth,
                right_num_log_tokens,
            );
        });

        // Recurse-via-looping to just the left subproblem.
        total_degrees = left_total_degrees;
        docs = left_docs;
    }
}

/// Create a CostTable big enough to hold any lookup for `partition`.
fn create_cost_table(total_degrees: &mut [AtomicU32]) -> CostTable {
    let max_degree = total_degrees
        .iter_mut()
        .map(|n| *n.get_mut())
        .max()
        .unwrap_or(0);

    // Add 1 because `compute_doc_move_gains` will try adding 1.
    CostTable::new(max_degree as usize + 1)
}

/// Permutes `docs` to be more compressible, a la
/// https://www.kdd.org/kdd2016/papers/files/rpp0883-dhulipalaAemb.pdf
pub fn optimize_doc_order(docs: &mut [Doc], config: &BalanceConfig) {
    if config.max_rounds == 0 || docs.len() <= 2 {
        return;
    }

    log::info!("Choosing balanced doc order with config {:?}", config);

    let num_timers = docs.len().next_power_of_two().trailing_zeros() as usize;
    let mut timers: Vec<Timers> = vec_with_default(num_timers);

    // Logging at every level as we recurse would be way too much, so instead
    // we use these log tokens to limit the total number of places we'll log while still
    // giving a rough idea what's going on at various recursion levels.
    let num_log_tokens = 4;

    let setup_timer = TimerGuard::new(&timers[0].setup_nanos);

    let num_rows = docs
        .par_iter()
        .map(|d| d.edge_list.last().map_or(0, |&n| n + 1))
        .max()
        .unwrap_or(0);

    let mut total_degrees = compute_degrees(docs, num_rows as usize, true);
    let cost_table = create_cost_table(&mut total_degrees);

    drop(setup_timer);

    rayon::scope(|s| {
        recursively_balance(
            s,
            docs,
            &cost_table,
            total_degrees,
            0,
            config,
            &timers,
            num_log_tokens,
        );
    });

    if log::log_enabled!(log::Level::Info) {
        // Delete timers for depths where we have no data.
        while timers.last().map_or(false, |t| t.is_empty()) {
            timers.pop();
        }

        for (depth, timer) in timers.iter().enumerate() {
            let title = if depth + 1 == num_timers {
                format!("depth >= {}", depth)
            } else {
                format!("depth {}", depth)
            };
            timer.log(&title);
        }

        log::info!("Choosing balanced doc order done");
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_reorder() {
        let config = BalanceConfig {
            max_par_depth: 2,
            min_num_docs: 8,
            max_leaf_window_size: 4,
            ..BalanceConfig::default()
        };

        fn create_weight(i: u32) -> u32 {
            if i % 37 != 0 { 1 } else { i % 7 }
        }

        // Create a dummy test case.
        let mut docs: Vec<Doc> = (0..1024)
            .map(|i| {
                let id = i as u32;
                let weight = create_weight(id);

                let edge_list: Vec<u32> = (0..32).filter(|j| (id >> j) & 1 != 0).collect();

                Doc {
                    edge_list,
                    weight,
                    id: ExternalId(id),
                }
            })
            .collect();

        optimize_doc_order(&mut docs, &config);

        // Guarantee it resulted in some permutation of what we gave it.
        let mut seen = vec![false; docs.len()];
        for doc in docs.iter() {
            assert_eq!(doc.weight, create_weight(doc.id.0));
            let old = std::mem::replace(&mut seen[doc.id.0 as usize], true);
            assert!(!old);
        }
    }
}
