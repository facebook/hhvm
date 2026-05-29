// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::OnceLock;

use oxidized::decl_parser_options::DeclParserOptions;

static THREAD_POOL: OnceLock<(usize, rayon::ThreadPool)> = OnceLock::new();

/// Return a lazily-initialized rayon thread pool whose worker threads use the
/// stack size from `opts.stack_size`. The pool is created on the first call
/// and reused on subsequent calls.
///
/// # Panics
/// * If `opts.stack_size < 4 MiB` (the parser needs at least this much).
/// * If two callers request different stack sizes (the pool can only be built once).
pub fn get_thread_pool(opts: &DeclParserOptions) -> &'static rayon::ThreadPool {
    let requested_stack_size = opts.stack_size as usize;

    // The direct decl parser is recursive-descent and allocates AST nodes on
    // the stack, so worker threads need a generous stack. As of December 2025,
    // 4 MiB is the minimum for the most deeply nested file in www.
    assert!(
        requested_stack_size >= 4 * 1024 * 1024,
        "requested_stack_size ({requested_stack_size}) must be >= 4 MiB"
    );

    let (pool_stack_size, pool) = THREAD_POOL.get_or_init(|| {
        let num_threads = std::thread::available_parallelism()
            .map(|n| n.get())
            .unwrap_or(4);
        let pool = rayon::ThreadPoolBuilder::new()
            .num_threads(num_threads)
            .stack_size(requested_stack_size)
            .build()
            .expect("Failed to create hack_thread_pool");
        (requested_stack_size, pool)
    });
    assert_eq!(
        *pool_stack_size, requested_stack_size,
        "Cannot use hack_thread_pool with varying stack sizes"
    );
    pool
}
