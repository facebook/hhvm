use std::time::Duration;
use std::time::Instant;

use once_cell::sync::Lazy;

static LOG_START: Lazy<Instant> = Lazy::new(Instant::now);
static LOG_LAST: Lazy<std::sync::RwLock<Instant>> =
    Lazy::new(|| std::sync::RwLock::new(Instant::now()));

fn log_durations() -> (Duration, Duration) {
    let mut last = LOG_LAST.write().unwrap();
    let now = Instant::now();
    let d1 = now.duration_since(*last);
    *last = now;
    drop(last);
    (d1, now.duration_since(*LOG_START))
}

/// Initialize a logger that shows the time since start and since the
/// previous log message, rather than absolute time.
pub fn init_delta_logger() {
    use std::io::Write;
    let mut builder = env_logger::Builder::from_default_env();

    // Be defensive against double-init by just throwing away the result.
    let _ = builder
        .format(|buf, record| {
            let (since_last, since_start) = log_durations();
            writeln!(
                buf,
                "[{:>6.2} {:>6.1} {}] {}",
                since_last.as_secs_f64(),
                since_start.as_secs_f64(),
                record.level(),
                record.args()
            )
        })
        .try_init();

    // Trigger lazy initialization.
    let _ = LOG_START.elapsed();
}
