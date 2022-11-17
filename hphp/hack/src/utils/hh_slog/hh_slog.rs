// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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

    let log = if binary_name.is_empty() {
        slog::Logger::root(drain, o!())
    } else {
        slog::Logger::root(drain, o!("bin" => binary_name))
    };
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
