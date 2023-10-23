// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ffi::c_void;
use std::ffi::CStr;
use std::ffi::CString;
use std::marker::PhantomData;
use std::os::raw::c_int;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;
use std::ptr;

use anyhow::Context;
use anyhow::Result;
use lmdb_sys::*;

// LMDB docs: http://www.lmdb.tech/doc/index.html

#[derive(Debug)]
pub struct Env {
    env: *mut MDB_env,
}

impl Env {
    /// Create a database in the directory specified by `path`.
    /// `path`: This directory must already exist and be writable.
    /// `size`: The size of the memory map to use for this environment,
    /// should be a multiple of the OS page size.
    /// `max_dbs`: Set the maximum number of named databases for the environment.
    /// Each named database is a logically separate key-value space.
    /// Even if this is set to zero, the unnamed database will be available.
    ///
    /// SAFETY: Only one instance per canonical path may exist process-wide.
    /// Do not have open an LMDB database twice in the same process at the same time.
    /// Not even from a plain open() call - close()ing it breaks flock() advisory locking.
    pub unsafe fn create(path: &Path, mapsize: usize, max_dbs: u32) -> Result<Self> {
        unsafe {
            let mut env = ptr::null_mut();
            check(mdb_env_create(&mut env))?;
            let env = Self { env };
            check(mdb_env_set_mapsize(env.env, mapsize))?;
            check(mdb_env_set_maxdbs(env.env, max_dbs))?;
            let path = CString::new(path.as_os_str().as_bytes())?;
            check(mdb_env_open(env.env, path.as_ptr(), 0, 0o644))?;
            Ok(env)
        }
    }

    pub fn rw_begin(&self) -> Result<RwTxn<'_>> {
        unsafe {
            let mut txn = ptr::null_mut();
            check(mdb_txn_begin(self.env, ptr::null_mut(), 0, &mut txn))?;
            let _env = PhantomData;
            Ok(RwTxn { txn, _env })
        }
    }

    pub fn ro_begin(&self) -> Result<RoTxn<'_>> {
        unsafe {
            let mut txn = ptr::null_mut();
            let flags = MDB_RDONLY;
            check(mdb_txn_begin(self.env, ptr::null_mut(), flags, &mut txn))?;
            let _env = PhantomData;
            Ok(RoTxn { txn, _env })
        }
    }

    pub fn create_db(&self, name: Option<&str>) -> Result<Db> {
        let txn = self.rw_begin()?;
        let c_name = name.map(|n| CString::new(n).unwrap());
        let c_name = c_name.as_ref().map_or(ptr::null(), |n| n.as_ptr());
        let mut dbi: MDB_dbi = 0;
        unsafe {
            check(mdb_dbi_open(txn.txn, c_name, MDB_CREATE, &mut dbi))
                .with_context(|| name.unwrap_or("").to_owned())?
        };
        txn.commit()?;
        Ok(Db(dbi))
    }

    pub fn page_size(&self) -> Result<usize> {
        let mut stat = std::mem::MaybeUninit::uninit();
        unsafe {
            check(mdb_env_stat(self.env, stat.as_mut_ptr()))?;
            Ok(stat.assume_init().ms_psize as usize)
        }
    }
}

// SAFETY: MDB_env is internally threadsafe
unsafe impl Sync for Env {}

// SAFETY: MDB_env is safe to transfer between threads
unsafe impl Send for Env {}

impl Drop for Env {
    fn drop(&mut self) {
        unsafe { mdb_env_close(self.env) }
    }
}

#[derive(Debug, Clone, Copy)]
pub struct Db(MDB_dbi);

pub trait Txn: Sized {
    fn txn(&self) -> *mut MDB_txn;

    fn commit(self) -> Result<()> {
        unsafe {
            let r = check(mdb_txn_commit(self.txn()));
            std::mem::forget(self);
            r
        }
    }

    fn get(&self, db: Db, key: impl AsRef<[u8]>) -> Result<Option<&[u8]>> {
        let mut k = slice_to_val(key);
        let mut data = zeroed_val();
        unsafe {
            match mdb_get(self.txn(), db.0, &mut k, &mut data) {
                MDB_NOTFOUND => Ok(None),
                status => check(status).map(|_| Some(val_to_slice(data))),
            }
        }
    }
}

pub struct RwTxn<'e> {
    txn: *mut MDB_txn,
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
    pub fn put(&self, db: Db, key: impl AsRef<[u8]>, value: impl AsRef<[u8]>) -> Result<()> {
        let mut k = slice_to_val(key);
        let mut v = slice_to_val(value);
        unsafe { check(mdb_put(self.txn, db.0, &mut k, &mut v, 0)) }
    }
}

pub struct RoTxn<'e> {
    txn: *mut MDB_txn,
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

impl<'e> RoTxn<'e> {
    pub fn ro_cursor(&self, db: Db) -> Result<RoCursor<'_>> {
        let mut cursor = ptr::null_mut();
        unsafe {
            check(mdb_cursor_open(self.txn, db.0, &mut cursor))?;
        }
        let _txn = PhantomData;
        Ok(RoCursor { cursor, _txn })
    }
}

pub struct RoCursor<'t> {
    #[allow(dead_code)]
    cursor: *mut MDB_cursor,
    _txn: PhantomData<fn() -> &'t ()>,
}

impl<'t> RoCursor<'t> {
    pub fn iter(&self) -> Iter<'t> {
        Iter::Ok {
            cursor: self.cursor,
            op: MDB_cursor_op_MDB_NEXT,
            next_op: MDB_cursor_op_MDB_NEXT,
            _mark: PhantomData,
        }
    }
}

pub enum Iter<'t> {
    Err(i32),
    Ok {
        cursor: *mut MDB_cursor,
        op: MDB_cursor_op,
        next_op: MDB_cursor_op,
        _mark: PhantomData<fn(_: &'t ())>,
    },
}

impl<'t> Iterator for Iter<'t> {
    type Item = Result<(&'t [u8], &'t [u8]), i32>;

    fn next(&mut self) -> Option<Self::Item> {
        match self {
            Self::Err(e) => Some(Err(*e)),
            Self::Ok {
                cursor,
                op,
                next_op,
                ..
            } => {
                let mut k = zeroed_val();
                let mut v = zeroed_val();
                let op = std::mem::replace(op, *next_op);
                unsafe {
                    match mdb_cursor_get(*cursor, &mut k, &mut v, op) {
                        0 => Some(Ok((val_to_slice(k), val_to_slice(v)))),
                        MDB_NOTFOUND => None,
                        e => Some(Err(e)),
                    }
                }
            }
        }
    }
}

unsafe fn val_to_slice<'a>(val: MDB_val) -> &'a [u8] {
    std::slice::from_raw_parts(val.mv_data as *const u8, val.mv_size)
}

fn slice_to_val(slice: impl AsRef<[u8]>) -> MDB_val {
    let slice = slice.as_ref();
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
        _ => unsafe {
            let e = CStr::from_ptr(mdb_strerror(status));
            Err(anyhow::anyhow!("{}", e.to_string_lossy()))
        },
    }
}

pub fn version() -> String {
    let null = ptr::null_mut();
    let v = unsafe { CStr::from_ptr(mdb_version(null, null, null)) };
    v.to_string_lossy().to_string()
}
