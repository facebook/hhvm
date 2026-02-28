// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! A simple telemetry crate ported from src/utils/core/measure.ml.
//!
//! The OCaml implementation will call into this one upon invocations of
//! `Measure.push_global` and `Measure.pop_global`, unioning the Rust
//! measurements into the returned OCaml record.
//!
//! The `measure` crate is primarily useful for debugging. It's particularly
//! useful for gathering stats about something that happens a lot. Let's say you
//! have some code like this
//!
//!     let number_bunnies = count_bunnies();
//!
//! If you want to debug how many bunnies are being counted, you could do
//! something like
//!
//!     let number_bunnies = count_bunnies();
//!     eprintln!("Num bunnies: {number_bunnies}");
//!
//! but what if this code is called 1000 times? Then you end up with log spew.
//! Using the `measure` crate helps with this. You can now do
//!
//!     let number_bunnies = count_bunnies();
//!     measure::sample("num_bunnies", number_bunnies);
//!
//! and then later you do
//!
//!     measure::print_stats();
//!
//! which will print the number of samples, the total, the average, the
//! variance, the max and the min.
//!
//! Measurements are stored in a stateful way in a record. You can either use a
//! global record or a local record.
//!
//! Using a global record:
//!
//!     measure::sample("num_bunnies", number_bunnies);
//!     measure::print_stats();
//!
//! You can push and pop the global record. This is useful if you want to reset
//! some counters without throwing away that data.
//!
//!     measure::push_global();
//!     // ...measure stuff
//!     let record = measure::pop_global();
//!     record.print_stats();
//!
//! Using a local record:
//!
//!     let record = measure::Record::default();
//!     record.sample("num_bunnies", number_bunnies);
//!     record.print_stats();
//!
//! A record does not store the individual measurements, just the aggregate
//! stats, which are updated online.

use ocamlrep::ToOcamlRep;
use once_cell::sync::Lazy;
use parking_lot::RwLock;

#[derive(Debug, Default)]
pub struct Record {
    entries: hash::DashMap<RecordName, RecordEntry>,
}

/// A `RecordName` can be constructed from a single static string which does not
/// contain the `(` character, or a pair of static strings where the first does
/// not contain the `(` character. When constructed from a pair, e.g.,
/// `RecordName::from(("foo", "bar"))`, the record name will be rendered as the
/// first string followed by the second, parenthesized, e.g., `"foo (bar)"`.
/// This is in order to support some existing patterns for constructing record
/// names in our OCaml without requiring the caller to concatenate strings at
/// sample time.
#[derive(Copy, Clone, Hash, PartialEq, Eq, PartialOrd, Ord)]
pub struct RecordName(&'static str, Option<&'static str>);

impl From<&'static str> for RecordName {
    fn from(name: &'static str) -> Self {
        debug_assert!(!name.contains('('));
        Self(name, None)
    }
}
impl From<(&'static str, &'static str)> for RecordName {
    fn from(names: (&'static str, &'static str)) -> Self {
        debug_assert!(!names.0.contains('('));
        Self(names.0, Some(names.1))
    }
}

static GLOBAL: Lazy<RwLock<Vec<Record>>> = Lazy::new(|| RwLock::new(vec![Default::default()]));

pub fn push_global() {
    GLOBAL.write().push(Default::default())
}

/// # Panics
///
/// Panics if invoked when the global record stack is empty.
pub fn pop_global() -> Record {
    match GLOBAL.write().pop() {
        Some(record) => record,
        None => panic!("measure::pop_global called with empty stack"),
    }
}

pub fn print_stats() {
    let stack = GLOBAL.read();
    let record = stack
        .last()
        .expect("No global record available! Did you forget to call measure::push_global?");
    record.print_stats()
}

/// # Panics
///
/// Panics if invoked when the global record stack is empty (i.e.,
/// `measure::pop_global` was called without a corresponding
/// `measure::push_global`).
#[inline]
pub fn sample(name: impl Into<RecordName>, value: f64) {
    let name: RecordName = name.into();
    sample_impl(name, value)
}
fn sample_impl(name: RecordName, value: f64) {
    let stack = GLOBAL.read();
    let record = stack
        .last()
        .expect("No global record available! Did you forget to call measure::push_global?");
    record.sample_impl(name, value)
}

impl Record {
    pub fn new() -> Self {
        Default::default()
    }

    #[inline]
    pub fn sample(&self, name: impl Into<RecordName>, value: f64) {
        let name: RecordName = name.into();
        self.sample_impl(name, value)
    }
    fn sample_impl(&self, name: RecordName, value: f64) {
        let mut entry = self.entries.entry(name).or_default();
        let RecordEntry {
            count: old_count,
            mean: old_mean,
            variance_sum,
            max,
            min,
            distribution: _,
        } = *entry;
        // The OCaml version allows different weights, but that feature seems to
        // be unused.
        let weight = 1.0;
        // Add `1 * weight` to the count
        let count = old_count + weight;
        let mean = old_mean + (weight * (value - old_mean) / count);
        // Knuth's online variance approximation algorithm, updated for weights.
        // Weighted version from http://people.ds.cam.ac.uk/fanf2/hermes/doc/antiforgery/stats.pdf
        let variance_sum = variance_sum + (weight * (value - old_mean) * (value - mean));
        let max = f64::max(max, value);
        let min = f64::min(min, value);
        *entry = RecordEntry {
            count,
            mean,
            variance_sum,
            max,
            min,
            distribution: (),
        };
    }

    pub fn print_stats(&self) {
        let mut entries: Vec<_> = (self.entries.iter())
            .map(|kv| (*kv.key(), *kv.value()))
            .collect();
        entries.sort_unstable_by_key(|&(name, _)| name);
        for (name, entry) in &entries {
            let prefix = format!("{name} stats --");
            if entry.count == 0.0 {
                eprintln!("{prefix} NO DATA");
            } else {
                let total = entry.count * entry.mean;
                let std_dev = (entry.variance_sum / entry.count).sqrt();
                eprintln!(
                    "{prefix} samples: {}, total: {}, avg: {}, stddev: {}, max: {}, min: {}",
                    pretty_num(entry.count),
                    pretty_num(total),
                    pretty_num(entry.mean),
                    pretty_num(std_dev),
                    pretty_num(entry.max),
                    pretty_num(entry.min),
                )
            }
        }
    }
}

fn pretty_num(f: f64) -> String {
    if f > 1000000000.0 {
        format!("{:.3}G", f / 1000000000.0)
    } else if f > 1000000.0 {
        format!("{:.3}M", f / 1000000.0)
    } else if f > 1000.0 {
        format!("{:.3}K", f / 1000.0)
    } else if f == f.floor() {
        format!("{}", f as u64)
    } else {
        format!("{}", f)
    }
}

#[derive(Copy, Clone, Debug, ToOcamlRep)]
struct RecordEntry {
    count: f64,
    mean: f64,
    variance_sum: f64,
    max: f64,
    min: f64,
    // Included only for the sake of the derived ToOcamlRep impl.
    // We're taking advantage here of the fact that `None` has the same
    // representation in OCaml as `unit` (since in OCaml this field has type
    // `distribution option`).
    distribution: (),
}

impl Default for RecordEntry {
    fn default() -> Self {
        Self {
            count: 0.0,
            mean: 0.0,
            variance_sum: 0.0,
            max: f64::MIN,
            min: f64::MAX,
            distribution: (),
        }
    }
}

impl std::fmt::Display for RecordName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if let Some(suffix) = self.1 {
            write!(f, "{} ({})", self.0, suffix)
        } else {
            write!(f, "{}", self.0)
        }
    }
}
impl std::fmt::Debug for RecordName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        format!("{self}").fmt(f)
    }
}

// Implemented manually instead of derived because in OCaml, records are keyed
// by a single string, and invokers of `Measure.sample` concatenate categories
// with parenthesized subcategories (since string concatenation can be done
// inexpensively in the minor heap). `RecordName` supports the same
// concatenation pattern without needing to do any allocation or memcpys at
// sample-time; we just have to do the concatenation when converting to OCaml
// instead.
impl ToOcamlRep for RecordName {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        use std::io::Write;
        let mut str = alloc.byte_string_with_len(self.0.len() + self.1.map_or(0, |s| s.len() + 3));
        write!(&mut str, "{self}").unwrap();
        str.build()
    }
}

// Implemented manually because the OCaml version contains a sorted map rather
// than a hash map.
impl ToOcamlRep for Record {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let mut entries: Vec<_> = (self.entries.iter())
            .map(|kv| (*kv.key(), *kv.value()))
            .collect();
        entries.sort_unstable_by_key(|&(name, _)| name);
        let len = entries.len();
        let mut iter = entries
            .into_iter()
            .map(|(name, entry)| (alloc.add_copy(name), alloc.add_copy(entry)));
        let (res, _) = ocamlrep::sorted_iter_to_ocaml_map(&mut iter, alloc, len);
        res
    }
}
