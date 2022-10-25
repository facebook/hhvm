// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(assert_matches)]

use std::os::unix::prelude::FileExt;
use std::path::Path;
use std::path::PathBuf;

/// This class provides a RwLock backed by file and flock(LOCK_EX/LOCK_SH), which
/// is therefore usable as a synchronization primitive by multiple processes.
/// * The file on disk may be in state "absent / present / poisoned / stopped".
/// * Poisoning means that unless a process released the lock cleanly,
///   then no other process can acquire it. Poisoning is important for multi-process
///   since having one process die via Ctrl+C is a routine event and has to be handled.
///   (As for whether poisoning is important for multi-thread, it only arises
///   when a thread panics, and some people think that behavior after a panic is unimportant).
/// * Unix flock is "advisory". This means that it is respected by anyone who
///   uses flock, but ignored by anyone who doesn't. Thus, someone can read+write
///   the the lock's file even while we consider it locked, just by bypassing RwLock.
///   You might consider that a feature or a bug! but there's nothing better available.
///   As a safeguard, though, we do mark the file with a "poisoned" byte while anyone
///   holds a lock, so that people won't inadvertently read it.
/// * With an in-memory lock you know that RwLock::new() will necessarily succeed,
///   but for a shared-file lock then FileRwLock::create() might fail in the case
///   that the lock already exists on disk. This is indicated by LockError::Present
///   Also, as a convenience, the create() method will create any directories necessary.
/// * There's no 'delete' method, no way provided by this class to remove the file
///   from disk. This reflects underlying OS race limitations. You will have to do
///   std::fs::remove_file() yourself at a time when you know there won't be races,
///   e.g. you know there won't be a concurrent "create".
/// * As implementation, the "T" is stored on disk as pretty json. That's to help
///   humans read it for debugging. It is deserialized when you lock(), and
///   serialized again when you release the lock. To help humans debugging,
///   create() also serializes it to disk. Also as implementation, the json is
///   stored on disk with a leading space, and that leading space is replaced
///   with "@" to indicate both exclusive-locking and poison. This is how FileRwLock
///   knows to fail a lock attempt it's poisoned, and it also prevents any non-FileRwLock
///   consumers from inadvertently json-parsing the file when it's locked or poisoned.
///   We mark the "@" immediately after acquiring the flock(LOCK_EX), and remove it
///   immediately before releasing.
///
/// IMPLEMENTATION
/// I'd like to say I copied this all from stackoverflow, but I couldn't find any file
/// locks on the internet that seemed rigorous enough. A bunch of internet places suggest
/// a two-file approach (file, file.lock). I struggled to see how that would provide
/// serializability in the presence of crashes, and also it felt unnecessarily clunky.
///
/// I had wanted to have a DELETE method instead of STOP, one that would unlink the file
/// off disk. But there are two problems with that.
/// First, consider "absent -> CREATE || DELETE;CREATE". The first CREATE might get in first
/// and grab a file descriptor, then DELETE;CREATE runs and unlinks the file and creates a
/// new file in its place and locks it, then the first CREATE continues with its own
/// private (now deleted) file descriptor. We can't have two concurrent CREATEs both holding
/// exclusive locks at the same time and both thinking they're the only ones. Therefore,
/// the first CREATE in this situation must fail, maybe by having the DELETE mark the
/// file as poisoned or deleted. But now we've invented a failure mode that's not serializable,
/// that arises solely when CREATE happens concurrently with DELETE.
/// Second, consider "present -> A.DELETE || B.DELETE;CREATE". Both A and B might open file-
/// descriptors to the file. B gets the lock first, poisons the file, and unlinks it.
/// B then creates a new file at the same directory location and returns an exclusive lock on it.
/// A then acquires a lock on the old (now unlinked) file, poisons it a second time,
/// and unlinks. The problem is that it poisoned the old version of the file but unlinked
/// the new file. The end result is that CREATE believes it has an exclusive lock,
/// but there exists no file on disk. This is an unavoidable race with the way unlink works:
/// https://bugzilla.kernel.org/show_bug.cgi?id=93441
///
/// I picked "empty file" to represent STOPPED. That just made some code neater in "stop".
///
/// The way I reasoned about this was a gigantic state diagram for two concurrent processes.
/// Every state has (FILE, STATE1, STATE2):
/// FILE ::= absent | empty | poisoned | value
/// STATE ::= c0 -create,flock,writep-> c1 -mv[RDWR|EX]-> c2[RDWR|EX] -writev-> c3 -close->
///           w0 -open-> w1[RDWR] -flock-> w2[RDWR|EX] -read-> c3[RDWR|EX] -writep -> w4[RDWR|EX] -writev-> w5 -close->
///           r0 -open-> r1[RD] -flock-> r2[RD|SH] -read-> r3[RD|SH] -close->
///           s0 -open-> s1[RDWR] -flock-> s2[RDWR|EX] -writes-> s3[RDWR|EX] -close->
/// The correctness statement for mutual exclusion is that there should be no reachable state with
/// (_, c2/w4/r3, c2/w4/r3) other than (_, r3, r3). The correctness statement for serializability is that for
/// every path starting at (_,,c0/w0/r0/s0,c0/w0/r0/d0) which leads to (_,c3/w5/r3/s3,c3/w5/r3/s3), there
/// exists another path which does all of process 1 followed by all of process 2 or vice versa.
/// Well, I managed to draw by the full state diagram for each of the four process kinds in isolation, and for all
/// pairs of processes kinds, but the diagrams grew large enough that I lost confidence that I was exhaustive.
/// Also, I didn't get on to more complex combinations like "WRITE || DELETE;CREATE".
#[derive(Debug, Clone)]
pub struct FileRwLock<T> {
    path: PathBuf,
    value_type: std::marker::PhantomData<T>,
}

/// This byte at the start of a file signifies that the lock is poisoned.
const POISONED: u8 = b'@';

/// This byte at the start of the file signifies that the lock is present
/// and has a value. We use a symbol that won't interfere with json parsing,
/// for human debugging convenience.
const PRESENT: u8 = b' ';

impl<T: serde::Serialize + for<'de> serde::Deserialize<'de>> FileRwLock<T> {
    /// This method doesn't do anything on disk to the lockfile.
    /// It merely initializes the FileRwLock<T> structure in memory, so you
    /// can do subsequent create/read/write/stop operations that modify disk.
    pub fn new(path: PathBuf) -> Self {
        Self {
            path,
            value_type: std::marker::PhantomData,
        }
    }

    /// Creates a new lockfile on disk, resulting in the file being in state "present"
    /// and with an exclusive lock on it.
    /// Will fail if the file was previously present/stopped/poisoned; all three
    /// cases are represented by CreateError::Present.
    pub fn create(
        &mut self,
        value: T,
    ) -> Result<Option<FileRwLockExclusiveGuard<'_, T>>, UnexpectedError> {
        if let Some(parent) = self.path.parent() {
            std::fs::create_dir_all(parent).path_context(&self.path, "create_dir_all")?;
        }

        // We will atomically create+initialize+lock the file. This will either succeed with us
        // having ownership, or will fail because the file already exists. How to achieve this?
        // (1) create+initialize+lock the file in some other location, then "mv" it into place
        // For this to work we'd need renameat2(RENAME_NOREPLACE) but this isn't portable
        // and indeed doesn't exist on some of hack's targets.
        // (2) create+initialize+lock the file in some temporary location on the same device, then
        // do link(...) to atomically link it from the desired path, then remove the temporary.
        // This works portably but it's a bit of a pain to have to remove the temporary, and might
        // leave dangling temporaries if we crash.
        // (3) create+initialize+lock the file using O_TMPFILE, then do linkat(fd,"",AT_FDCWD,path,AT_EMPTY_PATH)
        // to atomically place, https://stackoverflow.com/questions/17127522/create-a-hard-link-from-a-file-handle-on-unix/18644492#18644492
        // This would be nice but AT_EMPTY_PATH is gated by CAP_DAC_READ_SEARCH, which isn't in force
        // in some places we run. Also, O_TMPFILE is only on linux; not on bsd (mac) nor windows.
        // (4) as above using O_TMPFILE, but linkat(AT_FDCWD,"/proc/self/fd/{fd}",AT_FDCWD,path,AT_SYMLINK_FOLLOW)
        // This doesn't need the capability -- for why, see https://www.spinics.net/lists/linux-fsdevel/msg101911.html.
        // It still isn't portable because of O_TMPFILE. But this is the option I'll pick.

        // Because we're using link, we need to create the temp file on the same device.
        // open(O_TMPFILE) takes a directory argument for this reason, to know which device
        // to create on, and it's normal to use the desired containing directory.
        use std::os::unix::ffi::OsStrExt;
        let c_parent = std::ffi::CString::new(match self.path.parent() {
            None => &b"/"[..],
            Some(parent) => parent.as_os_str().as_bytes(),
        })
        .path_context(&self.path, "cparent")?;

        // Create and lock the file. This uses O_TMPFILE which isn't supported everywhere.
        let fd = unsafe { libc::open(c_parent.as_ptr(), libc::O_TMPFILE | libc::O_RDWR, 0o666) };
        if fd < 0 {
            return Err(std::io::Error::last_os_error()).path_context(&self.path, "open(O_TMPFILE");
        }
        use std::os::unix::io::FromRawFd;
        let mut file = unsafe { std::fs::File::from_raw_fd(fd) };
        use fs2::FileExt;
        file.lock_exclusive()
            .path_context(&self.path, "flock(LOCK_EX)")?;

        // Initialize the file. We'll put in poison so that if we crash then other processes
        // will know the file is in an indeterminate state. For human debuggability
        // of anyone viewing the lockfile, we'll write out the full initial value to it.
        use std::io::Write;
        let json =
            serde_json::to_string_pretty(&value).path_context(&self.path, "json::to_string")?;
        let content = format!("{}{json}", POISONED as char);
        file.write_all(content.as_bytes())
            .path_context(&self.path, "write_poison_initial")?;

        // Link the locked+initialized file into the desired location.
        // Our sequence point is the moment we link.
        let c_path = std::ffi::CString::new(self.path.as_os_str().as_bytes())
            .path_context(&self.path, "cpath")?;
        let c_proc_fd = std::ffi::CString::new(format!("/proc/self/fd/{fd}")).unwrap();
        #[cfg(not(target_os = "linux"))]
        compile_error!("This uses /proc/self/fd, which is linux-only");
        let ret = unsafe {
            libc::linkat(
                libc::AT_FDCWD,
                c_proc_fd.as_ptr(),
                libc::AT_FDCWD,
                c_path.as_ptr(),
                libc::AT_SYMLINK_FOLLOW,
            )
        };
        if ret >= 0 {
            // Over to the ExclusiveGuard, who will be responsible for writing
            // back a real (non-poisoned) value to it upon drop/close.
            Ok(Some(FileRwLockExclusiveGuard {
                lock: self,
                file: Some(file),
                value,
            }))
        } else {
            let e = std::io::Error::last_os_error();
            if e.kind() == std::io::ErrorKind::AlreadyExists {
                // Either it already existed prior to this 'create' even started,
                // or a concurrent 'create' raced with us and won.
                Ok(None)
            } else {
                Err(e).path_context(&self.path, "linkat")
            }
        }
    }

    // Internal helper function used by read/ write...
    // Opens the file in RDWR or RD mode depending on the flag,
    // locks it with LOCK_EX or LOCK_SH depending on the flag,
    // reads the content,
    // validates that it's present (i.e. not absent/stopped/poisoned),
    // deserializes it from json into T,
    // and returns the file and T.
    fn open_lock_read_validate(&self, write: bool) -> Result<(std::fs::File, T), LockError> {
        let context = if write { "open(RDWR)" } else { "open(RD)" };
        let mut file = match std::fs::OpenOptions::new()
            .read(true)
            .write(write)
            .open(&self.path)
        {
            Ok(file) => file,
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => {
                return Err(LockError::Absent(self.path.clone()));
            }
            Err(e) => {
                return Err(e)
                    .path_context(&self.path, context)
                    .map_err(LockError::Unexpected);
            }
        };

        // Our sequence point will be the moment we acquire the lock.
        use fs2::FileExt;
        if write {
            file.lock_exclusive()
                .path_context(&self.path, "flock(LOCK_EX)")?;
        } else {
            file.lock_shared()
                .path_context(&self.path, "flock(LOCKS_SH)")?;
        }

        // Now we have the lock, we can examine what we have in hand...
        use std::io::Read;
        let mut content = String::new();
        file.read_to_string(&mut content)
            .path_context(&self.path, "read")?;
        match content.bytes().next() {
            Some(PRESENT) => {
                let value: T =
                    serde_json::from_str(&content).path_context(&self.path, "json::from_str")?;
                Ok((file, value))
            }
            Some(POISONED) => Err(LockError::Poisoned(self.path.clone())),
            Some(_) => Err(anyhow::anyhow!("corrupt: {}", content))
                .path_context(&self.path, "validate")
                .map_err(LockError::Unexpected),
            None => Err(LockError::Stopped(self.path.clone())),
        }
    }

    /// Locks this lockfile with shared read access, blocking the current thread until it can
    /// can be acquired. The lockfile must be present -- if absent/poisoned/stopped, error.
    pub fn read(&self) -> Result<FileRwLockSharedGuard<'_, T>, LockError> {
        // Our sequence point is the moment the lock is acquired
        let (file, value) = self.open_lock_read_validate(true)?;

        // Over to the SharedGuard, who will be responsible for
        // closing the file-descriptor and hence releasing the lock, upon drop.
        Ok(FileRwLockSharedGuard {
            lock: self,
            file,
            value,
        })
    }

    /// Locks this lockfile with exclusive write access, blocking the current thread until it can
    /// can be acquired. The lockfile must be present -- if absent/poisoned/stopped, error.
    pub fn write(&mut self) -> Result<FileRwLockExclusiveGuard<'_, T>, LockError> {
        // Our sequence point is the moment the lock is acquired
        let (file, value) = self.open_lock_read_validate(true)?;

        // Now we'll poison the file. Prior to this point, if we crashed, then
        // everyone else would find the lock present and unclaimed, which is fine
        // since we've not yet returned the ExclusiveGuard to our caller. After this point
        // though, if we crash, everyone else will find it poisoned.
        file.write_all_at(&[POISONED], 0)
            .path_context(&self.path, "write(poison)")?;

        // Over to the ExclusiveGuard, who will be responsible for writing
        // back a real (non-poisoned) value to it upon drop/close, and also
        // closing the file-descriptor hence releasing the lock.
        Ok(FileRwLockExclusiveGuard {
            lock: self,
            file: Some(file),
            value,
        })
    }

    /// Puts the lockfile into a permanently stopped state. The only way out of this
    /// is to delete the path from disk and start over. This function works fine
    /// regardless of the state - present, absent, stopped, poisoned.
    pub fn stop(self) -> Result<(), UnexpectedError> {
        // If the file was initially absent, the stop method still has to mark the lockfile
        // as permanently stopped. We represent permanently-stopped with an empty file.
        // The following "open" will create an empty file if none existed.
        match std::fs::OpenOptions::new()
            .write(true)
            .create(true)
            .open(&self.path)
        {
            Err(e) => Err(e).path_context(&self.path, "open(WR)"),
            Ok(file) => {
                // Our sequence point will be the moment we acquire the lock.
                use fs2::FileExt;
                file.lock_exclusive()
                    .path_context(&self.path, "flock(LOCK_EX)")?;

                // A stopped lockfile is represented by an empty file -- either
                // empty from the create(true).open() statement above at the start
                // to "stop", or we empty it right here and now.
                file.set_len(0).path_context(&self.path, "trunc")?;
                Ok(())
            }
        }
    }
}

/// This structure is a Deref/DerefMut around 'value', but with the additional
/// property that upon drop/close then it serializes 'value' into the file
/// and closes the file (hence releasing all flocks).
#[derive(Debug)]
pub struct FileRwLockExclusiveGuard<'a, T: serde::Serialize> {
    lock: &'a FileRwLock<T>,
    file: Option<std::fs::File>,
    value: T,
}

impl<T: serde::Serialize> std::ops::Deref for FileRwLockExclusiveGuard<'_, T> {
    type Target = T;
    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

impl<T: serde::Serialize> std::ops::DerefMut for FileRwLockExclusiveGuard<'_, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.value
    }
}

impl<'a, T: serde::Serialize> FileRwLockExclusiveGuard<'a, T> {
    fn close_impl(&mut self) -> Result<(), UnexpectedError> {
        let file = match self.file.take() {
            None => return Ok(()),
            Some(file) => file,
        };

        // Our caller might have called deref_mut and mutated self.value.
        // Now, upon close, is the moment when we'll write back that modified value.
        // (until now the file had merely been marked as poisoned).
        // We might imagine alternative models, e.g. (1) we only add the poison
        // marker upon the first call to deref_mut although that's hard because
        // deref_mut isn't supposed to have errors, (2) we optimize to avoid the
        // writeback if deref_mut never got called, (3) we allow the caller
        // to write into the file while they hold the lock rather than waiting
        // until the end, for debugging purposes. But until we find a clear
        // need for such models, we'll stick with the current simple approach.
        let json = serde_json::to_string_pretty(&self.value)
            .path_context(&self.lock.path, "json::to_string")?;

        // We must be sure upon crash we won't leave the file either
        // empty or with partially written content but no poison marker. So: we'll write
        // the value starting at byte offset 1, trim any off the end in case the lockfile
        // has shrunk, and only after that is finished will we remove the poison pill at
        // byte offset 0.
        file.write_all_at(json.as_bytes(), 1)
            .path_context(&self.lock.path, "write(value@1)")?;
        // do the truncate here, after writing json, so as to minimize how much we
        // jiggle the file's size.
        file.set_len(1 + json.as_bytes().len() as u64)
            .path_context(&self.lock.path, "trunc")?;
        file.write_all_at(&[PRESENT], 0)
            .path_context(&self.lock.path, "write(value@0)")?;

        // As for the job of closing file-descriptors in self.file?
        // Well, we can leave that to the normal drop machinery,
        // which happens right here and now!
        Ok(())
    }

    pub fn close(mut self) -> Result<(), UnexpectedError> {
        self.close_impl()
    }
}

impl<T: serde::Serialize> Drop for FileRwLockExclusiveGuard<'_, T> {
    fn drop(&mut self) {
        let _ = self.close_impl();
    }
}

/// This structure is a Deref around 'value', but with the additional property
/// that upon drop/close then it closes the file (hence releasing all flocks)
#[derive(Debug)]
pub struct FileRwLockSharedGuard<'a, T: serde::Serialize> {
    /// This lock parameter isn't actually used, but it expresses that the lock
    /// has a 'borrow' for the duration of the shared lock. This is just a convenience
    /// so that some detection of "tried to acquire exclusive lock while shared
    /// lock is held" can be done at compile-time, rather than solely at run-time.
    #[allow(unused)]
    lock: &'a FileRwLock<T>,
    /// This file parameter isn't actually used directly; it's only used because
    /// when the struct is dropped then the file will be dropped too, and with it
    /// the OS will drop all locks. If we didn't keep this parameter then the file
    /// and its locks would be dropped too soon.
    #[allow(unused)]
    file: std::fs::File,
    value: T,
}

impl<T: serde::Serialize> std::ops::Deref for FileRwLockSharedGuard<'_, T> {
    type Target = T;
    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

/// Errors that might arise from FileRwLock::read/write
#[derive(thiserror::Error, Debug)]
pub enum LockError {
    #[error("{} absent", .0.display())]
    Absent(PathBuf),
    #[error("{} poisoned", .0.display())]
    Poisoned(PathBuf),
    #[error("{} stopped", .0.display())]
    Stopped(PathBuf),
    #[error(transparent)]
    Unexpected(#[from] UnexpectedError),
}

/// Errors that might arise from FileRwLock::create/stop/close
#[derive(thiserror::Error, Debug)]
pub enum UnexpectedError {
    #[error("{} - {} - {}", .0.display(), .1, .2)]
    Unexpected(PathBuf, &'static str, #[source] anyhow::Error),
}

trait UnexpectedContext<T> {
    fn path_context(self, path: &Path, context: &'static str) -> Result<T, UnexpectedError>;
}

impl<T, E: Into<anyhow::Error>> UnexpectedContext<T> for Result<T, E> {
    fn path_context(self, path: &Path, context: &'static str) -> Result<T, UnexpectedError> {
        self.map_err(|e| UnexpectedError::Unexpected(path.to_owned(), context, e.into()))
    }
}

#[cfg(test)]
mod tests {
    use std::assert_matches::assert_matches;

    use super::*;

    #[test]
    fn test_create_then_lock() -> anyhow::Result<()> {
        let tmpdir = tempfile::tempdir()?;

        // with an integer
        let path = tmpdir.path().join("lock1");
        let mut lock = FileRwLock::new(path.clone());
        let mut guard = lock.create(15)?.expect("no create conflicts");
        assert_eq!(*guard, 15);
        assert_eq!(std::fs::read_to_string(&path)?, "@15");
        *guard = 12;
        guard.close()?;
        assert_eq!(std::fs::read_to_string(&path)?, " 12");
        let guard = lock.write()?;
        assert_eq!(std::fs::read_to_string(&path)?, "@12");
        assert_eq!(*guard, 12);
        guard.close()?;
        assert_eq!(std::fs::read_to_string(&path)?, " 12");

        // with unit, to check that our "poison" byte is ok
        let path = tmpdir.path().join("lock2");
        let mut lock = FileRwLock::new(path.clone());
        let guard = lock.create(())?.expect("no create conflicts");
        assert_eq!(std::fs::read_to_string(&path)?, "@null");
        guard.close()?;

        // with a string, to check that it looks right with our poison byte
        let path = tmpdir.path().join("lock3");
        let mut lock = FileRwLock::new(path.clone());
        let guard = lock.create("!".to_owned())?.expect("no create conflicts");
        assert_eq!(*guard, "!");
        assert_eq!(std::fs::read_to_string(&path)?, "@\"!\"");
        guard.close()?;

        Ok(())
    }

    #[test]
    fn test_cannot_create() -> anyhow::Result<()> {
        let tmpdir = tempfile::tempdir()?;

        // can't create if created
        let path = tmpdir.path().join("lock1");
        let mut lock = FileRwLock::new(path.clone());
        let guard = lock.create(16)?.expect("no create conflicts");
        guard.close()?;
        assert!(lock.create(16)?.is_none());

        // can't create if stopped
        lock.stop()?;
        let mut lock = FileRwLock::new(path);
        assert!(lock.create(17)?.is_none());

        // can't create if poisoned
        let path = tmpdir.path().join("lock2");
        std::fs::write(&path, "@12")?;
        let mut lock = FileRwLock::new(path);
        assert!(lock.create(18)?.is_none());

        Ok(())
    }

    #[test]
    fn test_cannot_lock() -> anyhow::Result<()> {
        let tmpdir = tempfile::tempdir()?;

        // can't lock if doesn't exist
        let path = tmpdir.path().join("lock1");
        let mut lock = FileRwLock::<i32>::new(path);
        assert_matches!(lock.write(), Err(LockError::Absent(_)));

        // can't lock if it's stopped
        let path = tmpdir.path().join("lock2");
        let lock = FileRwLock::<i32>::new(path.clone());
        lock.stop()?;
        let mut lock = FileRwLock::<i32>::new(path);
        assert_matches!(lock.write(), Err(LockError::Stopped(_)));

        // can't lock if it's poisoned
        let path = tmpdir.path().join("lock3");
        std::fs::write(&path, "@123\n")?;
        let mut lock = FileRwLock::<i32>::new(path);
        assert_matches!(lock.write(), Err(LockError::Poisoned(_)));

        Ok(())
    }

    #[test]
    fn test_lock_will_wait() -> anyhow::Result<()> {
        let tmpdir = tempfile::tempdir()?;
        let path = tmpdir.path().join("lock");
        let mut lock = FileRwLock::new(path.clone());
        let mut guard = lock.create(15)?.expect("no create conflict");

        let path2 = path;
        let thread = std::thread::spawn(move || -> anyhow::Result<_> {
            let mut lock2 = FileRwLock::<i32>::new(path2);
            let guard2 = lock2.write()?;
            Ok(*guard2)
        });
        std::thread::sleep(std::time::Duration::from_millis(100));
        *guard = 16;
        guard.close()?;
        assert_eq!(thread.join().unwrap()?, 16);
        Ok(())
    }
}
