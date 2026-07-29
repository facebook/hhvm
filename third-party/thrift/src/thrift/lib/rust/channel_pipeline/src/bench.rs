/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! Rust-owned benchmark binary for synchronous channel_pipeline integration.

use channel_pipeline as _;

#[cxx::bridge(namespace = "channel_pipeline_rust::bench")]
mod ffi {
    struct BenchResult {
        adapter_round_trip_ns: f64,
        native_read_pipeline_ns: f64,
        rust_read_pipeline_ns: f64,
        native_write_pipeline_ns: f64,
        rust_write_pipeline_ns: f64,
        native_exception_pipeline_ns: f64,
        rust_exception_pipeline_ns: f64,
        native_read_ready_ns: f64,
        rust_read_ready_ns: f64,
        native_read_recovery_ns: f64,
        rust_read_recovery_ns: f64,
        native_write_recovery_ns: f64,
        rust_write_recovery_ns: f64,
        ready_path_alloc_bytes: u64,
        forward_path_alloc_bytes: u64,
        ready_path_loop_callbacks: u64,
        forward_path_loop_callbacks: u64,
        jemalloc_available: bool,
    }

    unsafe extern "C++" {
        include!("thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustBenchHarness.h");

        fn run_bench_with_watchdog(iterations: u64, timeout_ms: u64) -> BenchResult;
    }
}

fn main() {
    const ITERATIONS: u64 = 100_000;
    const REPETITIONS: usize = 10;
    const TIMEOUT_MS: u64 = 30_000;

    let mut samples = Vec::with_capacity(REPETITIONS);
    for repetition in 0..REPETITIONS {
        let result = ffi::run_bench_with_watchdog(ITERATIONS, TIMEOUT_MS);
        println!(
            "raw repetition={repetition} adapter_round_trip_ns={:.3} native_read_pipeline_ns={:.3} rust_read_pipeline_ns={:.3} native_write_pipeline_ns={:.3} rust_write_pipeline_ns={:.3} native_exception_pipeline_ns={:.3} rust_exception_pipeline_ns={:.3}",
            result.adapter_round_trip_ns,
            result.native_read_pipeline_ns,
            result.rust_read_pipeline_ns,
            result.native_write_pipeline_ns,
            result.rust_write_pipeline_ns,
            result.native_exception_pipeline_ns,
            result.rust_exception_pipeline_ns,
        );
        println!(
            "raw repetition={repetition} native_read_ready_ns={:.3} rust_read_ready_ns={:.3} native_read_recovery_ns={:.3} rust_read_recovery_ns={:.3} native_write_recovery_ns={:.3} rust_write_recovery_ns={:.3} ready_path_alloc_bytes={} forward_path_alloc_bytes={} ready_path_loop_callbacks={} forward_path_loop_callbacks={} jemalloc_available={}",
            result.native_read_ready_ns,
            result.rust_read_ready_ns,
            result.native_read_recovery_ns,
            result.rust_read_recovery_ns,
            result.native_write_recovery_ns,
            result.rust_write_recovery_ns,
            result.ready_path_alloc_bytes,
            result.forward_path_alloc_bytes,
            result.ready_path_loop_callbacks,
            result.forward_path_loop_callbacks,
            result.jemalloc_available,
        );
        samples.push(result);
    }

    print_stats("adapter_round_trip_ns", &samples, |sample| {
        sample.adapter_round_trip_ns
    });
    print_stats("native_read_pipeline_ns", &samples, |sample| {
        sample.native_read_pipeline_ns
    });
    print_stats("rust_read_pipeline_ns", &samples, |sample| {
        sample.rust_read_pipeline_ns
    });
    print_stats("native_write_pipeline_ns", &samples, |sample| {
        sample.native_write_pipeline_ns
    });
    print_stats("rust_write_pipeline_ns", &samples, |sample| {
        sample.rust_write_pipeline_ns
    });
    print_stats("incremental_read_handler_ns", &samples, |sample| {
        sample.rust_read_pipeline_ns - sample.native_read_pipeline_ns
    });
    print_stats("incremental_write_handler_ns", &samples, |sample| {
        sample.rust_write_pipeline_ns - sample.native_write_pipeline_ns
    });
    print_stats("native_exception_pipeline_ns", &samples, |sample| {
        sample.native_exception_pipeline_ns
    });
    print_stats("rust_exception_pipeline_ns", &samples, |sample| {
        sample.rust_exception_pipeline_ns
    });
    print_stats("native_read_ready_ns", &samples, |sample| {
        sample.native_read_ready_ns
    });
    print_stats("rust_read_ready_ns", &samples, |sample| {
        sample.rust_read_ready_ns
    });
    print_stats("incremental_read_ready_ns", &samples, |sample| {
        sample.rust_read_ready_ns - sample.native_read_ready_ns
    });
    print_stats("native_read_recovery_ns", &samples, |sample| {
        sample.native_read_recovery_ns
    });
    print_stats("rust_read_recovery_ns", &samples, |sample| {
        sample.rust_read_recovery_ns
    });
    print_stats("incremental_read_recovery_ns", &samples, |sample| {
        sample.rust_read_recovery_ns - sample.native_read_recovery_ns
    });
    print_stats("native_write_recovery_ns", &samples, |sample| {
        sample.native_write_recovery_ns
    });
    print_stats("rust_write_recovery_ns", &samples, |sample| {
        sample.rust_write_recovery_ns
    });
    print_stats("incremental_write_recovery_ns", &samples, |sample| {
        sample.rust_write_recovery_ns - sample.native_write_recovery_ns
    });

    // Allocation and enqueue evidence is deterministic; report the observed
    // maxima so any nonzero value on the normal synchronous path is visible.
    let ready_alloc = samples
        .iter()
        .map(|sample| sample.ready_path_alloc_bytes)
        .max()
        .unwrap_or(0);
    let forward_alloc = samples
        .iter()
        .map(|sample| sample.forward_path_alloc_bytes)
        .max()
        .unwrap_or(0);
    let ready_cbs = samples
        .iter()
        .map(|sample| sample.ready_path_loop_callbacks)
        .max()
        .unwrap_or(0);
    let forward_cbs = samples
        .iter()
        .map(|sample| sample.forward_path_loop_callbacks)
        .max()
        .unwrap_or(0);
    let jemalloc = samples.iter().all(|sample| sample.jemalloc_available);
    println!(
        "evidence jemalloc_available={jemalloc} ready_path_alloc_bytes_max={ready_alloc} forward_path_alloc_bytes_max={forward_alloc} ready_path_loop_callbacks_max={ready_cbs} forward_path_loop_callbacks_max={forward_cbs}"
    );
}

fn print_stats(
    name: &str,
    samples: &[ffi::BenchResult],
    select: impl Fn(&ffi::BenchResult) -> f64,
) {
    let mut values: Vec<_> = samples.iter().map(select).collect();
    values.sort_by(f64::total_cmp);
    let mean = values.iter().sum::<f64>() / values.len() as f64;
    let variance = values
        .iter()
        .map(|value| (value - mean).powi(2))
        .sum::<f64>()
        / values.len() as f64;
    let median = (values[values.len() / 2 - 1] + values[values.len() / 2]) / 2.0;
    println!(
        "stats {name} n={} min={:.3} median={median:.3} mean={mean:.3} stddev={:.3} max={:.3}",
        values.len(),
        values[0],
        variance.sqrt(),
        values[values.len() - 1],
    );
}
