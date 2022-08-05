// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use fs2::FileExt;
use slog::Drain;

/// Locks the file when logging. Prevents multi-process writes from mangling if
/// every process follows the advisory lock (flock on Unix).
pub struct LockedFileDrain {
    filepath: std::path::PathBuf,
}

impl LockedFileDrain {
    pub fn new(filepath: &std::path::Path) -> Self {
        Self {
            filepath: filepath.to_path_buf(),
        }
    }
}

impl Drain for LockedFileDrain {
    type Ok = ();
    // A requirement to be used with Async. Since Async would process the logs
    // in a separate thread, we wouldn't be able to handle the error anyway.
    type Err = slog::Never;

    fn log(
        &self,
        record: &slog::Record<'_>,
        logger_values: &slog::OwnedKVList,
    ) -> Result<Self::Ok, Self::Err> {
        // We acquire a new file handle each time we log to ensure the file
        // exists when logging. This is because when keeping a file handle
        // across logs and the log file gets deleted, we will quietly fail to
        // log to the file.
        let file = match std::fs::OpenOptions::new()
            .create(true)
            .append(true)
            .open(&self.filepath)
        {
            Ok(file) => file,
            Err(err) => {
                eprintln!("Unable to get log file handle. Failed to log. Err: {}", err);
                return Ok(());
            }
        };

        // Acquiring exclusive lock
        while let Err(err) = file.lock_exclusive() {
            match err.kind() {
                // This shouldn't happen often, but if it's just an
                // interruption, retry.
                std::io::ErrorKind::Interrupted => continue,
                _ => {
                    eprintln!(
                        "failed to acquire exclusive lock: {}. Logging anyway...",
                        err
                    );
                    break;
                }
            }
        }

        let drain = slog_json::Json::default(&file);
        if let Err(err) = drain.log(record, logger_values) {
            eprintln!("LockedFileDrain failed to log: {}", err);
        }

        // Releasing lock
        while let Err(err) = file.unlock() {
            match err.kind() {
                std::io::ErrorKind::Interrupted => continue,
                _ => {
                    eprintln!("failed to release exclusive lock: {}", err);
                    break;
                }
            }
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::io::BufRead;

    use serde_json::Value;
    use slog::o;
    use tempfile::NamedTempFile;

    use super::*;

    #[test]
    fn log_to_file() {
        let temp_logfile = NamedTempFile::new().unwrap();
        let drain = LockedFileDrain::new(temp_logfile.path());
        let logger = slog::Logger::root(drain.fuse(), o!());

        slog::info!(logger, "test msg!"; "test_key" => 1);
        slog::info!(logger, "test msg 2!"; "test_key" => 2);

        let mut lines = std::io::BufReader::new(temp_logfile)
            .lines()
            .map(|l| l.unwrap());

        let test_msg_line = lines.next().unwrap();
        if let Ok(Value::Object(json)) = serde_json::from_str(&test_msg_line) {
            assert_eq!(json.get("msg").unwrap(), &Value::String("test msg!".into()));
            assert_eq!(json.get("test_key").unwrap(), &Value::Number(1.into()));
        } else {
            panic!("unable to parse logfile line: {}", &test_msg_line);
        }

        let test_msg_line2 = lines.next().unwrap();
        if let Ok(Value::Object(json)) = serde_json::from_str(&test_msg_line2) {
            assert_eq!(
                json.get("msg").unwrap(),
                &Value::String("test msg 2!".into())
            );
            assert_eq!(json.get("test_key").unwrap(), &Value::Number(2.into()));
        } else {
            panic!("unable to parse logfile line: {}", &test_msg_line2);
        }
    }
}
