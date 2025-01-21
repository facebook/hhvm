// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::marker::PhantomData;

use anyhow::Context;
use anyhow::Result;

use super::Db;
use super::Env;
use super::RoCursor;
use super::RwTxn;
use super::Txn;

#[derive(Clone, Copy)]
pub struct TypedDb<K> {
    db: Db,
    mark: PhantomData<K>,
}

impl<K> TypedDb<K> {
    fn new(db: Db) -> Self {
        Self {
            db,
            mark: PhantomData,
        }
    }
}

fn strip_prefix_ignore_case<'k>(prefix: &[u8], k: &'k [u8]) -> Option<&'k [u8]> {
    let i = prefix.len();
    if k.len() < i {
        None
    } else if prefix.eq_ignore_ascii_case(&k[..i]) {
        Some(&k[i..])
    } else {
        None
    }
}

impl<'k, K: Encode<'k>> TypedDb<K> {
    pub fn create(env: &Env, name: Option<&str>) -> Result<Self> {
        let db = env.create_db(name, K::INTKEY, K::IGNORE_CASE)?;
        Ok(Self::new(db))
    }

    pub fn open(env: &Env, name: Option<&str>) -> Result<Self> {
        let db = env.open_db(name, K::INTKEY, K::IGNORE_CASE)?;
        Ok(Self::new(db))
    }

    pub fn get<'t>(&self, txn: &'t impl Txn, k: &'k K::Item) -> Result<Option<&'t [u8]>> {
        txn.get(self.db, K::encode(k))
    }

    pub fn ro_cursor<'t>(&self, txn: &'t impl Txn) -> Result<RoCursor<'t>> {
        txn.ro_cursor(self.db)
    }

    /// Return an iterator over k,v pairs where k starts with prefix.
    /// The yeilded items have the prefix removed from the key.
    // XXX use K::Item, and only for string slices
    pub fn iter_prefix<'t, 'p>(
        &self,
        txn: &'t impl Txn,
        prefix: &'p [u8],
    ) -> Result<impl Iterator<Item = Result<(&'t [u8], &'t [u8])>> + 'p> {
        let c = txn.ro_cursor(self.db)?;
        Ok(c.iter_from(prefix).map_while(|e| match e {
            Err(e) => Some(Err(e)),
            Ok((k, v)) => {
                let suffix = if K::IGNORE_CASE {
                    strip_prefix_ignore_case(prefix, k)
                } else {
                    k.strip_prefix(prefix)
                };
                suffix.map(|suffix| Ok((suffix, v)))
            }
        }))
    }

    pub fn put(&self, txn: &mut RwTxn<'_>, k: &'k K::Item, data: impl AsRef<[u8]>) -> Result<()> {
        txn.put(self.db, K::encode(k), data)
    }

    pub fn append(
        &self,
        txn: &mut RwTxn<'_>,
        k: &'k K::Item,
        data: impl AsRef<[u8]>,
    ) -> Result<()> {
        txn.append(self.db, K::encode(k), data)
    }

    pub fn del(&self, txn: &mut RwTxn<'_>, k: &'k K::Item) -> Result<()> {
        txn.del(self.db, K::encode(k))
    }

    pub fn len(&self, txn: &impl Txn) -> Result<usize> {
        txn.num_entries(self.db)
    }

    pub fn is_empty(&self, txn: &impl Txn) -> Result<bool> {
        self.len(txn).map(|n| n == 0)
    }
}

impl<K> TypedDb<K> {
    pub fn bulk_append<'k, I, V>(&self, txn: &mut RwTxn<'_>, items: I) -> Result<()>
    where
        I: Iterator<Item = (&'k K::Item, V)>,
        K: Encode<'k>,
        V: AsRef<[u8]>,
    {
        for (i, (k, data)) in items.enumerate() {
            txn.append(self.db, K::encode(k), data)
                .with_context(|| format!("{i}"))?;
        }
        Ok(())
    }
}

pub trait Encode<'a> {
    const INTKEY: bool;
    const IGNORE_CASE: bool;
    type Item: ?Sized + 'a;
    fn encode(item: &'a Self::Item) -> &'a [u8];
}

#[derive(Clone, Copy)]
pub struct Usize;
impl<'a> Encode<'a> for Usize {
    const INTKEY: bool = true;
    const IGNORE_CASE: bool = false;
    type Item = usize;
    fn encode(item: &'a usize) -> &'a [u8] {
        unsafe { std::mem::transmute::<&usize, &[u8; 8]>(item) }
    }
}

#[derive(Clone, Copy)]
pub struct VecU8;
impl<'a> Encode<'a> for VecU8 {
    const INTKEY: bool = false;
    const IGNORE_CASE: bool = false;
    type Item = Vec<u8>;
    fn encode(item: &'a Vec<u8>) -> &'a [u8] {
        item.as_slice()
    }
}

#[derive(Clone, Copy)]
pub struct Str;
impl<'a> Encode<'a> for Str {
    const INTKEY: bool = false;
    const IGNORE_CASE: bool = false;
    type Item = str;
    fn encode(item: &'a str) -> &'a [u8] {
        item.as_bytes()
    }
}

#[derive(Clone, Copy)]
pub struct StrNoCase;
impl<'a> Encode<'a> for StrNoCase {
    const INTKEY: bool = false;
    const IGNORE_CASE: bool = true;
    type Item = str;
    fn encode(item: &'a str) -> &'a [u8] {
        item.as_bytes()
    }
}

#[derive(Clone, Copy)]
pub struct OsStr;
impl<'a> Encode<'a> for OsStr {
    const INTKEY: bool = false;
    const IGNORE_CASE: bool = false;
    type Item = std::ffi::OsStr;
    fn encode(item: &'a std::ffi::OsStr) -> &'a [u8] {
        std::os::unix::ffi::OsStrExt::as_bytes(item)
    }
}
