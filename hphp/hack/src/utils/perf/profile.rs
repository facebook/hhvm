extern crate libc;

use std::mem;

/// Gets CPU times for this and all child processes.
/// Returns
///     (this proc user, this proc sys,
///      this thread user, this thread sys,
///      children user, children sys)
///
/// Adapted from OCaml implementation in
///  ocaml/otherlibs/unix/times.c
pub fn get_cpu_time_seconds() -> (f64, f64, f64, f64, f64, f64) {
    let mut ru_self: libc::rusage = unsafe { mem::zeroed() };
    let mut ru_thread: libc::rusage = unsafe { mem::zeroed() };
    let mut ru_children: libc::rusage = unsafe { mem::zeroed() };

    const RUSAGE_SELF: i32 = 0;
    const RUSAGE_THREAD: i32 = 1;
    const RUSAGE_CHILDREN: i32 = -1;

    unsafe {
        libc::getrusage(RUSAGE_SELF, &mut ru_self);
        libc::getrusage(RUSAGE_THREAD, &mut ru_thread);
        libc::getrusage(RUSAGE_CHILDREN, &mut ru_children);
    }

    let to_seconds = |t: libc::timeval| t.tv_sec as f64 + t.tv_usec as f64 / 1e6;
    (
        to_seconds(ru_self.ru_utime),
        to_seconds(ru_self.ru_stime),
        to_seconds(ru_thread.ru_utime),
        to_seconds(ru_thread.ru_stime),
        to_seconds(ru_children.ru_utime),
        to_seconds(ru_children.ru_stime),
    )
}

pub fn get_user_thread_cpu_time_seconds() -> f64 {
    get_cpu_time_seconds().2
}

pub fn get_sys_cpu_time_seconds() -> f64 {
    let times = get_cpu_time_seconds();
    times.1 + times.5
}

/// Given a function to profile, profiles it multiple times such that it runs at least min_time
/// seconds, returning the average time and the number of runs. Does not yet handle failures.
pub fn profile_longer_than<F>(mut f: F, min_time: f64, min_runs: u64) -> (f64, u64)
where
    F: FnMut(),
{
    let start = get_user_thread_cpu_time_seconds();
    let mut now = start;

    let mut iteration = 0;
    while (now - start) < min_time || iteration < min_runs {
        f();
        now = get_user_thread_cpu_time_seconds();
        iteration += 1;
    }

    return (
        (now - start) / (if iteration == 0 { 1. } else { iteration as f64 }),
        iteration,
    );
}

#[cfg(test)]
mod test {
    use super::*;
    use std::time::Instant;

    #[test]
    fn test_profile_longer_than() {
        let real_time_before = Instant::now();
        let (avg_user_time, nbr_runs) = profile_longer_than(
            || {
                for _ in 0..1000000 {
                    // Do some work.
                    let values = [1., 2., 3., 4., 5.];
                    if values.iter().sum::<f64>() == 0. {
                        panic!("Never hit, prevent loop from being optimized away");
                    }
                }
            },
            0.01,
            0,
        );
        let duration = Instant::now()
            .duration_since(real_time_before)
            .as_secs_f64();

        assert!(
            avg_user_time >= 1e-8,
            "time run shorter than given duration"
        );
        assert!(
            avg_user_time * nbr_runs as f64 >= 0.00001 * duration,
            "CPU time much less than real time"
        );
        assert!(
            avg_user_time * nbr_runs as f64 <= 10. * duration,
            "CPU time much more than real time"
        );
        assert!(nbr_runs > 0, "Loop never run");
    }

    #[test]
    fn test_profile_longer_than_min_runs_gt1() {
        let min_runs = 7;
        let mut cnt = 0;
        let (_, nbr_runs) = profile_longer_than(
            || {
                cnt = cnt + 1;
            },
            0.0,
            min_runs,
        );

        assert_eq!(nbr_runs, min_runs);
        assert_eq!(cnt, min_runs);
    }
}
