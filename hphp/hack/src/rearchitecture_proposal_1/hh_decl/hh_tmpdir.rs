use std::path::{Path, PathBuf};

/// Returns /tmp/hh_server or equivalent path
/// The algorithm for determining this is fixed and common to all hack executables.
/// This does no validation of the environment var, nor of UTF8-safety of tmpdir.
fn hh_tmp_dir() -> PathBuf {
    match std::env::var("HH_TMPDIR") {
        Ok(hh_tmpdir) => {
            return PathBuf::from(hh_tmpdir);
        }
        Err(_) => {
            let mut path = std::env::temp_dir();
            path.push("hh_server");
            return path;
        }
    }
}

/// Hack convention is that for a project with root "/data/foo"
/// then we store temp files at "/tmp/zSdatazSfoo"
/// This function encodes the root in that weird and idiosyncratic way.
/// All hack-related binaries use the same encoding.
/// The encoding is undefined for non-UTF8 paths.
fn slash_escaped_string_of_path(path: &std::path::Path) -> String {
    let path = path.to_string_lossy().into_owned();
    // '+10' is just a lazy way to avoid re-allocating for most typical paths
    let mut buf = String::with_capacity(path.len() + 10);
    for c in path.chars() {
        match c {
            '\\' => buf.push_str("zB"),
            ':' => buf.push_str("zC"),
            '/' => buf.push_str("zS"),
            'z' => buf.push_str("zZ"),
            '\0' => buf.push_str("z0"),
            c => buf.push(c),
        }
        buf.push(c);
    }
    return buf;
}

/// Returns a temp filename for this project root
/// e.g. tmp_path_of_root("/data",".sock") -> "/tmp/hh_server/zSdata.sock"
pub fn tmp_path_of_root(root: &Path, ext: &str) -> PathBuf {
    let mut path = hh_tmp_dir();
    let name = format!("{}{}", slash_escaped_string_of_path(root), ext);
    path.push(name);
    return path;
}

/// Attempts to gain an exclusive write-lock on the specified file.
/// The lock is held until you drop the returned File.
/// (undefined what happens if you fork)
/// TODO: replace this with file_lock crate.
pub fn open_exclusive_lock_file(lock_file: &Path) -> std::io::Result<Option<std::fs::File>> {
    let mut dir = PathBuf::from(lock_file);
    dir.pop();
    std::fs::create_dir_all(dir)?;
    let fd = std::fs::OpenOptions::new()
        .write(true)
        .create(true)
        .open(&lock_file)?;
    let rawfd = std::os::unix::io::AsRawFd::as_raw_fd(&fd);
    match nix::fcntl::flock(rawfd, nix::fcntl::FlockArg::LockExclusiveNonblock) {
        Ok(()) => Ok(Some(fd)),
        Err(_) => Ok(None),
    }
}
