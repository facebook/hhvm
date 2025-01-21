// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod typed;
use std::cmp::Ordering;
use std::ffi::c_void;
use std::ffi::CStr;
use std::ffi::CString;
use std::ffi::OsString;
use std::marker::PhantomData;
use std::os::raw::c_char;
use std::os::raw::c_int;
use std::os::raw::c_uint;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;
use std::ptr;
use std::sync::Arc;

use anyhow::Context;
use anyhow::Result;
use lmdb_sys::*;
pub use typed::*;

// LMDB docs: http://www.lmdb.tech/doc/index.html

/// A builder wrapping an unopened lmdb::Env, used for setting configuration
/// items that must be set before the environment is opened. Call Env::options()
/// to create one.
pub struct Options {
    env: *mut MDB_env,
    flags: c_uint,
}

#[derive(Debug)]
pub struct Env {
    env: Arc<EnvInner>,
}

#[derive(Debug)]
struct EnvInner(*mut MDB_env);
impl std::ops::Deref for EnvInner {
    type Target = *mut MDB_env;
    fn deref(&self) -> &*mut MDB_env {
        &self.0
    }
}

impl Drop for EnvInner {
    fn drop(&mut self) {
        unsafe { mdb_env_close(self.0) }
    }
}

// TODO Available Env flags:
// MDB_RDONLY - change api to open/create, ala File. write txn will fail

impl Options {
    pub fn set_mapsize(self, mapsize: usize) -> Result<Self> {
        unsafe {
            check(mdb_env_set_mapsize(self.env, mapsize))?;
        }
        Ok(self)
    }

    pub fn set_max_named_dbs(self, n: u32) -> Result<Self> {
        unsafe {
            check(mdb_env_set_maxdbs(self.env, n))?;
        }
        Ok(self)
    }

    fn set_flag(mut self, enable: bool, flag: c_uint) -> Self {
        match enable {
            false => self.flags &= !flag,
            true => self.flags |= flag,
        }
        self
    }

    /// Use a writable memory map if the Env is opened as writable.
    /// This is faster and uses fewer mallocs, but loses protection from
    /// application bugs like wild pointer writes and other bad updates
    /// into the database. Incompatible with nested transactions.
    /// Do not mix processes with and without MDB_WRITEMAP on the same environment.
    /// This can defeat durability (mdb_env_sync etc).
    ///
    /// Using this option will expand the database file sparsely to the size
    /// set by `set_mapsize()`. Actual storage will be only what is needed.
    /// To see the file size and usage, use `ll -ash <file>.mdb`.
    pub fn set_write_mmap(self, enable: bool) -> Self {
        self.set_flag(enable, MDB_WRITEMAP)
    }

    /// If true, Don't use thread-local storage for read transactions.
    /// Tie reader locktable slots to Txn objects instead of to threads.
    /// Txn::reset() keeps the slot reseved for the Txn object.
    /// A thread may use parallel read-only transactions. A read-only
    /// transaction may span threads if the user synchronizes its use.
    /// Applications that multiplex many user threads over individual OS threads
    /// need this option.
    pub fn set_notls(self, enable: bool) -> Self {
        self.set_flag(enable, MDB_NOTLS)
    }

    /// If true, don't zero init malloc'd memory before writing to file.
    /// Nop with set_write_mmap(true).
    pub fn set_nomeminit(self, enable: bool) -> Self {
        self.set_flag(enable, MDB_NOMEMINIT)
    }

    /// Turn off readahead. Most operating systems perform readahead on read requests by default. This option turns it off if the OS supports it. Turning it off may help random read performance when the DB is larger than RAM and system RAM is full. The option is not implemented on Windows.
    pub fn set_nordahead(self, enable: bool) -> Self {
        self.set_flag(enable, MDB_NORDAHEAD)
    }

    /// Set the maximum number of threads/reader slots for the environment.
    /// This defines the number of slots in the lock table that is used to
    /// track readers in the the environment. The default is 126. Starting
    /// a read-only transaction normally ties a lock table slot to the
    /// current thread until the environment closes or the thread exits.
    /// If MDB_NOTLS is in use, mdb_txn_begin() instead ties the slot to
    /// the MDB_txn object until it or the MDB_env object is destroyed.
    pub fn set_maxreaders(self, readers: u32) -> Result<Self> {
        unsafe {
            check(mdb_env_set_maxreaders(self.env, readers))?;
        }
        Ok(self)
    }

    /// Flush system buffers to disk only once per transaction, omit the
    /// metadata flush. Defer that until the system flushes files to disk,
    /// or next non-MDB_RDONLY commit or mdb_env_sync(). This optimization
    /// maintains database integrity, but a system crash may undo the last
    /// committed transaction. I.e. it preserves the ACI (atomicity, consistency,
    /// isolation) but not D (durability) database property.
    pub fn set_nometasync(self, enable: bool) -> Self {
        self.set_flag(enable, MDB_NOMETASYNC)
    }

    /// Create a database environment in the file specified by `path`.
    ///
    /// SAFETY: Only one instance per canonical path may exist process-wide.
    /// Do not have open an LMDB database twice in the same process at the same time.
    /// Not even from a plain open() call - close()ing it breaks flock() advisory locking.
    pub fn create_file(self, path: impl AsRef<Path>, perms: u16) -> Result<Env> {
        let Self { env, flags } = self;
        unsafe {
            let path = CString::new(path.as_ref().as_os_str().as_bytes())?;
            check(mdb_env_open(
                env,
                path.as_ptr(),
                flags | MDB_NOSUBDIR,
                perms.into(),
            ))?;
            std::mem::forget(self); // don't close our MDB_env
            Ok(Env {
                env: Arc::new(EnvInner(env)),
            })
        }
    }

    /// Open a database environment
    pub fn open_file(self, path: impl AsRef<Path>, perms: u16) -> Result<Env> {
        let Self { env, flags } = self;
        unsafe {
            let path = CString::new(path.as_ref().as_os_str().as_bytes())?;
            check(mdb_env_open(
                env,
                path.as_ptr(),
                flags | MDB_RDONLY | MDB_NOSUBDIR,
                perms.into(),
            ))?;
            std::mem::forget(self);
            Ok(Env {
                env: Arc::new(EnvInner(env)),
            })
        }
    }
}

impl Env {
    pub fn options() -> Result<Options> {
        unsafe {
            let mut env = ptr::null_mut();
            check(mdb_env_create(&mut env))?;
            Ok(Options { env, flags: 0 })
        }
    }

    pub fn rw_begin(&self) -> Result<RwTxn<'_>> {
        unsafe {
            let mut txn = ptr::null_mut();
            check(mdb_txn_begin(self.ptr(), ptr::null_mut(), 0, &mut txn))?;
            let _env = PhantomData;
            Ok(RwTxn { txn, _env })
        }
    }

    pub fn ro_begin(&self) -> Result<RoTxn<'_>> {
        unsafe {
            let mut txn = ptr::null_mut();
            let flags = MDB_RDONLY;
            check(mdb_txn_begin(self.ptr(), ptr::null_mut(), flags, &mut txn))?;
            let _env = PhantomData;
            Ok(RoTxn { txn, _env })
        }
    }

    pub fn snapshot(&self) -> Result<Snapshot> {
        unsafe {
            let mut txn = ptr::null_mut();
            let flags = MDB_RDONLY;
            check(mdb_txn_begin(self.ptr(), ptr::null_mut(), flags, &mut txn))?;
            let _env = Arc::clone(&self.env);
            Ok(Snapshot { txn, _env })
        }
    }

    /// Create a logical key/value space (db) within the environment.
    /// There can be one unnamed the unnamed "main" db, and additional
    /// named tables. The number of named tables is limited by
    /// Options::set_max_named_dbs(), zero by default.
    pub(crate) fn create_db(
        &self,
        name: Option<&str>,
        intkey: bool,
        ignore_case: bool,
    ) -> Result<Db> {
        let txn = self.rw_begin()?;
        let db = Self::open(&txn, name, intkey, ignore_case, MDB_CREATE)?;
        txn.commit()?;
        Ok(db)
    }

    /// Get the Db handle for an existing named or unnamed db.
    /// Fail if it doesn't already exist.
    pub(crate) fn open_db(
        &self,
        name: Option<&str>,
        intkey: bool,
        ignore_case: bool,
    ) -> Result<Db> {
        Self::open(&self.ro_begin()?, name, intkey, ignore_case, 0)
    }

    fn open(
        txn: &impl Txn,
        name: Option<&str>,
        intkey: bool,
        ignore_case: bool,
        mut flags: u32,
    ) -> Result<Db> {
        // TODO mutex around this
        let c_name = name.map(|n| CString::new(n).unwrap());
        let c_name = c_name.as_ref().map_or(ptr::null(), |n| n.as_ptr());
        if intkey {
            flags |= MDB_INTEGERKEY;
        }
        let mut dbi: MDB_dbi = 0;
        unsafe {
            check(mdb_dbi_open(txn.txn(), c_name, flags, &mut dbi))
                .with_context(|| name.unwrap_or("").to_owned())?;
            if ignore_case {
                check(mdb_set_compare(txn.txn(), dbi, Some(nocase_compare)))?;
            }
        }
        Ok(Db(dbi))
    }

    pub fn page_size(&self) -> Result<usize> {
        let mut stat = std::mem::MaybeUninit::uninit();
        unsafe {
            check(mdb_env_stat(self.ptr(), stat.as_mut_ptr()))?;
            Ok(stat.assume_init().ms_psize as usize)
        }
    }

    pub fn max_key_size(&self) -> usize {
        unsafe { mdb_env_get_maxkeysize(self.ptr()) as u32 as usize }
    }

    pub fn clean_readers(&self) -> Result<usize> {
        let mut dead = 0;
        unsafe { check(mdb_reader_check(self.ptr(), &mut dead))? };
        Ok(dead as usize)
    }

    fn ptr(&self) -> *mut MDB_env {
        **self.env
    }

    fn paths(&self) -> Result<(OsString, OsString)> {
        let path = unsafe {
            let mut cpath: *const c_char = ptr::null_mut();
            check(mdb_env_get_path(self.ptr(), &mut cpath))?;
            std::ffi::OsStr::from_bytes(CStr::from_ptr(cpath).to_bytes())
        };
        let mut lockpath = path.to_os_string();
        lockpath.push("-lock");
        Ok((path.to_os_string(), lockpath))
    }

    /// Change unix permissions on env and lock file
    pub fn chmod(&self, mode: u16) -> Result<()> {
        use std::os::unix::fs::PermissionsExt;
        let (path, lockpath) = self.paths()?;
        std::fs::set_permissions(path, PermissionsExt::from_mode(mode.into()))?;
        std::fs::set_permissions(lockpath, PermissionsExt::from_mode(mode.into()))?;
        Ok(())
    }

    /// Change ownership of env and lock file
    pub fn chown(&self, uid: Option<u32>, gid: Option<u32>) -> Result<()> {
        let (path, lockpath) = self.paths()?;
        std::os::unix::fs::chown(path, uid, gid)?;
        std::os::unix::fs::chown(lockpath, uid, gid)?;
        Ok(())
    }
}

impl Drop for Options {
    fn drop(&mut self) {
        unsafe { mdb_env_close(self.env) }
    }
}

// SAFETY: MDB_env is internally threadsafe
unsafe impl Sync for EnvInner {}

// SAFETY: MDB_env is safe to transfer between threads
unsafe impl Send for EnvInner {}

// TODO: Available DB options
// MDB_REVERSEKEY    compare keys as reversed strings
// MDB_DUPSORT       treat (key,value) as key, subject to max_key_size
// MDB_DUPFIXED      fixed-size (key,value) items. requires DUPSORT
// MDB_INTEGERDUP    (key,value) are both size_t
// MDB_REVERSEDUP    compare data items as reversed strings
#[allow(dead_code)]
struct DbConfig {
    name: Option<String>,
    intkey: bool,
}

#[derive(Debug, Clone, Copy)]
pub struct Db(MDB_dbi);

const TOUPPER: [u8; 256] = [
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
    /* a->A */ 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
    85, 86, 87, 88, 89, /* z->Z */ 90, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133,
    134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152,
    153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171,
    172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
    191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228,
    229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255,
];

// SAFETY: a and b must point to valid MDB_vals, which must themselves
// point to valid data slices, for the lifetime of this function.
unsafe extern "C" fn nocase_compare(a: *const MDB_val, b: *const MDB_val) -> i32 {
    let a = val_to_slice(*a);
    let b = val_to_slice(*b);
    for i in 0..std::cmp::min(a.len(), b.len()) {
        let d = TOUPPER[a[i] as usize] as i32 - TOUPPER[b[i] as usize] as i32;
        if d != 0 {
            return d;
        }
    }
    match a.len().cmp(&b.len()) {
        Ordering::Less => -1,
        Ordering::Equal => 0,
        Ordering::Greater => 1,
    }
}

#[inline]
pub fn memcasecmp(a: &[u8], b: &[u8]) -> Ordering {
    // SAFETY: a and b are valid slices for the lifetime of the comparison
    match unsafe { nocase_compare(&slice_to_val(a), &slice_to_val(b)) } {
        x if x < 0 => Ordering::Less,
        0 => Ordering::Equal,
        _ => Ordering::Greater,
    }
}

pub trait Txn: Sized {
    fn txn(&self) -> *mut MDB_txn;

    fn commit(self) -> Result<()> {
        unsafe {
            let r = check(mdb_txn_commit(self.txn()));
            std::mem::forget(self);
            r
        }
    }

    fn abort(self) {
        unsafe { mdb_txn_abort(self.txn()) };
        std::mem::forget(self);
    }

    fn get(&self, db: Db, key: impl AsRef<[u8]>) -> Result<Option<&[u8]>> {
        let mut k = slice_to_val(key.as_ref());
        let mut data = zeroed_val();
        unsafe {
            match mdb_get(self.txn(), db.0, &mut k, &mut data) {
                MDB_NOTFOUND => Ok(None),
                status => check(status).map(|_| Some(val_to_slice(data))),
            }
        }
    }

    fn ro_cursor(&self, db: Db) -> Result<RoCursor<'_>> {
        let mut cursor = ptr::null_mut();
        unsafe {
            check(mdb_cursor_open(self.txn(), db.0, &mut cursor))?;
        }
        let _txn = PhantomData;
        Ok(RoCursor { cursor, _txn })
    }

    fn num_entries(&self, db: Db) -> Result<usize> {
        let mut stat = std::mem::MaybeUninit::uninit();
        unsafe {
            check(mdb_stat(self.txn(), db.0, stat.as_mut_ptr()))?;
            Ok(stat.assume_init().ms_entries)
        }
    }
}

pub struct RwTxn<'e> {
    txn: *mut MDB_txn,

    // zero sized marker that constrains this RwTxn to its Env lifetime
    _env: PhantomData<&'e ()>,
}

impl<'e> Drop for RwTxn<'e> {
    fn drop(&mut self) {
        unsafe { mdb_txn_abort(self.txn) }
    }
}

impl<'e> Txn for RwTxn<'e> {
    fn txn(&self) -> *mut MDB_txn {
        self.txn
    }
}

impl<'e> RwTxn<'e> {
    /// Fail with MDB_BAD_VALSIZE if a key length is outside [1,max_key_size()]
    /// or value exceeds 4GiB. Zero length keys are not allowed.
    pub fn put(&mut self, db: Db, key: impl AsRef<[u8]>, value: impl AsRef<[u8]>) -> Result<()> {
        let mut k = slice_to_val(key.as_ref());
        let mut v = slice_to_val(value.as_ref());
        unsafe { check(mdb_put(self.txn, db.0, &mut k, &mut v, 0)) }
    }

    pub fn append(&mut self, db: Db, key: impl AsRef<[u8]>, value: impl AsRef<[u8]>) -> Result<()> {
        let mut k = slice_to_val(key.as_ref());
        let mut v = slice_to_val(value.as_ref());
        unsafe { check(mdb_put(self.txn, db.0, &mut k, &mut v, MDB_APPEND)) }
    }

    pub fn bulk_append<I, K, V>(&mut self, db: Db, items: I) -> Result<()>
    where
        I: Iterator<Item = (K, V)>,
        K: AsRef<[u8]>,
        V: AsRef<[u8]>,
    {
        for (i, (key, data)) in items.enumerate() {
            self.append(db, key, data).with_context(|| format!("{i}"))?;
        }
        Ok(())
    }

    /// Insert (key, value) and return the old value if the key already existed.
    pub fn insert(
        &mut self,
        db: Db,
        key: impl AsRef<[u8]>,
        value: impl AsRef<[u8]>,
    ) -> Result<Option<Vec<u8>>> {
        // TODO use mdb_cursor_get + mdb_cursor_put
        let key = key.as_ref();
        let value = value.as_ref();
        let mut k = slice_to_val(key);
        let mut v = slice_to_val(value);
        unsafe {
            match mdb_put(self.txn, db.0, &mut k, &mut v, MDB_NOOVERWRITE) {
                0 => Ok(None),
                MDB_KEYEXIST => {
                    let old = val_to_slice(v).to_vec();
                    k = slice_to_val(key);
                    v = slice_to_val(value);
                    check(mdb_put(self.txn, db.0, &mut k, &mut v, 0))?;
                    Ok(Some(old))
                }
                e => check(e).map(|_| None),
            }
        }
    }

    pub fn del(&mut self, db: Db, key: impl AsRef<[u8]>) -> Result<()> {
        let mut k = slice_to_val(key.as_ref());
        unsafe { check(mdb_del(self.txn, db.0, &mut k, ptr::null_mut())) }
    }
}

pub struct RoTxn<'e> {
    txn: *mut MDB_txn,

    // zero sized marker that constrains this RoTxn to its Env lifetime
    _env: PhantomData<&'e ()>,
}

impl<'e> Drop for RoTxn<'e> {
    fn drop(&mut self) {
        unsafe { mdb_txn_abort(self.txn) }
    }
}

impl<'e> Txn for RoTxn<'e> {
    fn txn(&self) -> *mut MDB_txn {
        self.txn
    }
}

/// A readonly Txn that keeps the outer Env alive
/// This snapshot is not Send: it has threadlocal state tied to the thread
/// that created it.
/// It is also not Sync: only the creating thread may access it.
#[derive(Debug)]
pub struct Snapshot {
    txn: *mut MDB_txn,
    _env: Arc<EnvInner>,
}

impl Drop for Snapshot {
    fn drop(&mut self) {
        unsafe { mdb_txn_abort(self.txn) }
    }
}

impl Txn for Snapshot {
    fn txn(&self) -> *mut MDB_txn {
        self.txn
    }
}

// TODO: Are RoCursors tied to read transactions allowed to outlive the txn?
pub struct RoCursor<'t> {
    cursor: *mut MDB_cursor,

    // Zero sized marker that constrains this cursor to its Txn lifetime
    _txn: PhantomData<fn() -> &'t ()>,
}

impl<'t> RoCursor<'t> {
    pub fn iter(self) -> Iter<'t> {
        Iter::Ok {
            cursor: self.into_inner(),
            op: MDB_cursor_op_MDB_NEXT,
            next: MDB_cursor_op_MDB_NEXT,
            _mark: PhantomData,
        }
    }

    pub fn iter_from(self, key: impl AsRef<[u8]>) -> Iter<'t> {
        let key = key.as_ref();
        match self.set_range(key) {
            Ok(_) | Err(MDB_NOTFOUND) => Iter::Ok {
                cursor: self.into_inner(),
                op: MDB_cursor_op_MDB_GET_CURRENT,
                next: MDB_cursor_op_MDB_NEXT,
                _mark: PhantomData,
            },
            Err(e) => Iter::Err(e),
        }
    }

    /// Return an iterator over k,v pairs where k starts with prefix.
    /// The yeilded items have the prefix removed from the key.
    pub fn iter_prefix<'p>(
        self,
        prefix: &'p [u8],
    ) -> impl Iterator<Item = Result<(&'t [u8], &'t [u8])>> + 'p {
        self.iter_from(prefix).map_while(|e| match e {
            Err(e) => Some(Err(e)),
            Ok((k, v)) => k.strip_prefix(prefix).map(|suffix| Ok((suffix, v))),
        })
    }

    fn into_inner(self) -> *mut MDB_cursor {
        let cursor = self.cursor;
        std::mem::forget(self);
        cursor
    }

    fn set_range(&self, key: &[u8]) -> Result<(), c_int> {
        let mut k = slice_to_val(key);
        let mut d = zeroed_val();
        unsafe {
            check2(mdb_cursor_get(
                self.cursor,
                &mut k,
                &mut d,
                MDB_cursor_op_MDB_SET_RANGE,
            ))
        }
    }
}

impl<'t> Drop for RoCursor<'t> {
    fn drop(&mut self) {
        unsafe { mdb_cursor_close(self.cursor) }
    }
}

#[derive(Debug)]
pub enum Iter<'t> {
    Err(c_int),
    Ok {
        cursor: *mut MDB_cursor,
        op: MDB_cursor_op,
        next: MDB_cursor_op,
        _mark: PhantomData<fn(_: &'t ())>,
    },
}

impl<'t> Drop for Iter<'t> {
    fn drop(&mut self) {
        if let Self::Ok { cursor, .. } = self {
            unsafe { mdb_cursor_close(*cursor) }
        }
    }
}

impl<'t> Iterator for Iter<'t> {
    type Item = Result<(&'t [u8], &'t [u8])>;

    fn next(&mut self) -> Option<Self::Item> {
        match self {
            Self::Err(e) => Some(Err(status_to_err(*e))),
            Self::Ok {
                cursor, op, next, ..
            } => {
                let mut k = zeroed_val();
                let mut v = zeroed_val();
                let op = std::mem::replace(op, *next);
                unsafe {
                    match mdb_cursor_get(*cursor, &mut k, &mut v, op) {
                        0 => Some(Ok((val_to_slice(k), val_to_slice(v)))),
                        MDB_NOTFOUND | libc::EINVAL => None,
                        e => Some(Err(status_to_err(e))),
                    }
                }
            }
        }
    }
}

unsafe fn val_to_slice<'a>(val: MDB_val) -> &'a [u8] {
    std::slice::from_raw_parts(val.mv_data as *const u8, val.mv_size)
}

fn slice_to_val(slice: &[u8]) -> MDB_val {
    MDB_val {
        mv_size: slice.len(),
        mv_data: slice.as_ptr() as *mut c_void,
    }
}

fn zeroed_val() -> MDB_val {
    MDB_val {
        mv_size: 0,
        mv_data: ptr::null_mut(),
    }
}

fn check(status: c_int) -> Result<()> {
    match status {
        0 => Ok(()),
        _ => Err(status_to_err(status)),
    }
}

fn status_to_err(status: c_int) -> anyhow::Error {
    anyhow::anyhow!("{}", status_to_string(status))
}

fn status_to_string(status: c_int) -> String {
    let cstr = unsafe { CStr::from_ptr(mdb_strerror(status)) };
    cstr.to_string_lossy().into_owned()
}

fn check2(status: c_int) -> Result<(), c_int> {
    match status {
        0 => Ok(()),
        e => Err(e),
    }
}

pub fn version() -> String {
    let null = ptr::null_mut();
    let v = unsafe { CStr::from_ptr(mdb_version(null, null, null)) };
    v.to_string_lossy().to_string()
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn put_copies_inputs() -> Result<()> {
        let tmpdir = tempfile::tempdir()?;
        for mmap_mode in [false, true] {
            let path = tmpdir.path().join(format!("test-{mmap_mode}.mdb"));
            let env = Env::options()?
                .set_write_mmap(mmap_mode)
                .create_file(path, 0o666)?;
            let main = env.create_db(None, false, false)?;
            let mut txn = env.rw_begin()?;
            let mut v = Vec::new();
            let mut s = String::new();
            for i in 0..100 {
                s.push_str(&format!("{i:30}"));
                // repeatedly store slices of the same string with different contents
                txn.put(main, s.as_bytes(), [])?;
                v.push(s.clone());
                s.clear();
            }
            txn.commit()?;
            v.sort_unstable(); // lmdb sorts byte strings lexicographically
            let txn = env.ro_begin()?;
            let mut n = 0;
            for (e, s) in txn.ro_cursor(main)?.iter().zip(v.iter()) {
                let (k, _) = e.unwrap();
                // did put() copy its input slices?
                assert_eq!(k, s.as_bytes(), "oops {s}");
                n += 1;
            }
            assert_eq!(n, v.len());
        }
        tmpdir.close()?;
        Ok(())
    }

    #[test]
    fn check_toupper() {
        for (i, c) in TOUPPER.iter().enumerate() {
            assert_eq!(TOUPPER[i], c.to_ascii_uppercase());
            assert_eq!(TOUPPER[i], (i as u8).to_ascii_uppercase());
        }
    }
}
