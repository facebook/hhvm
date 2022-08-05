// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg_attr(not(fbcode_build), allow(unused_variables))]

use slog::o;
use slog::Drain;

const TIMESTAMP_FORMAT: &str = "[%Y-%m-%d %H:%M:%S%.3f]";

fn timestamp_format(io: &mut dyn std::io::Write) -> std::io::Result<()> {
    write!(io, "{}", chrono::Local::now().format(TIMESTAMP_FORMAT))
}

#[derive(Clone, Debug)]
pub struct Log {
    /// Logs to both scuba and the log file.
    pub scuba: slog::Logger,
    /// Only logs to the log file.
    pub file: slog::Logger,
}

/// Initializes a `Log` for the specified scuba dataset and log file path, and
/// returns its async guards (scuba guard, log file guard).
/// When dropped, async guards will send a flush+terminate message to all loggers
/// derived from its corresponding logger in `Log`.
pub fn init(
    fb: fbinit::FacebookInit,
    binary_name: &'static str,
    dataset: &str,
    filename: &std::path::Path,
) -> (Log, (slog_async::AsyncGuard, slog_async::AsyncGuard)) {
    // The scuba logger just becomes a file logger in non-fbcode builds
    #[cfg(fbcode_build)]
    let (scuba, scuba_guard) = hh_slog_scuba::init_scuba_logfile(fb, dataset, filename);
    #[cfg(not(fbcode_build))]
    let (scuba, scuba_guard) = init_file(filename);

    let (file, file_guard) = init_file(filename);
    let scuba = scuba.new(o!("bin" => binary_name));
    let file = file.new(o!("bin" => binary_name));

    (Log { scuba, file }, (scuba_guard, file_guard))
}

/// Creates a logger that drains to the given path. Also returns its guard (read
/// `init` for more details).
/// The file logger uses unix flock to lock the log file when flushing its logs.
/// With this overhead, the throughput is still around 7200 short messages per
/// second.
pub fn init_file(filename: &std::path::Path) -> (slog::Logger, slog_async::AsyncGuard) {
    let drain = locked_file_drain::LockedFileDrain::new(filename);
    // NB: panicking in the async thread does not cause Fuse to panic.
    let (drain, guard) = slog_async::Async::new(drain)
        .thread_name("slog_logfile".to_owned())
        .build_with_guard();

    let log = slog::Logger::root(drain.fuse(), o!());
    (log, guard)
}

pub fn init_term(binary_name: &'static str) -> (slog::Logger, slog_async::AsyncGuard) {
    let decorator = slog_term::TermDecorator::new().build();

    let drain = slog_term::FullFormat::new(decorator)
        .use_custom_timestamp(timestamp_format)
        .build()
        .fuse();

    let drain = slog_envlogger::new(drain);

    let (drain, guard) = slog_async::Async::new(drain)
        .thread_name("slog_async".to_owned())
        .build_with_guard();

    let drain = drain.fuse();

    let log = slog::Logger::root(drain, o!("bin" => binary_name));
    (log, guard)
}

pub fn init_term_testing() -> slog::Logger {
    // Due to how cargo and buck capture stdout, we have to use a special sink
    // to display stdout in tests.
    let decorator = slog_term::PlainSyncDecorator::new(slog_term::TestStdoutWriter);
    let drain = slog_term::FullFormat::new(decorator)
        .use_custom_timestamp(timestamp_format)
        .build()
        .fuse();

    slog::Logger::root(drain, o!())
}

/// Initializes a `Log` where each logger prints to stdout synchronously.
/// Used for testing.
pub fn init_testing() -> Log {
    Log {
        scuba: init_term_testing(),
        file: init_term_testing(),
    }
}

/// Wrapper around `hh_slog_scuba::init_scuba` that defaults to a test (terminal) logger in non-fb builds.
pub fn init_scuba(
    fb: fbinit::FacebookInit,
    dataset: &str,
) -> (slog::Logger, slog_async::AsyncGuard) {
    #[cfg(fbcode_build)]
    return hh_slog_scuba::init_scuba(fb, dataset);
    #[cfg(not(fbcode_build))]
    return init_term("not-fbcode-build-scuba");
}

/// Wrapper around `hh_slog_scuba::init_scuba_sync` that defaults to a test
/// (terminal) logger in non-fb builds.
pub fn init_scuba_sync(fb: fbinit::FacebookInit, dataset: &str) -> slog::Logger {
    #[cfg(fbcode_build)]
    return hh_slog_scuba::init_scuba_sync(fb, dataset);
    #[cfg(not(fbcode_build))]
    return init_term_testing();
}
