// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(allocator_api)]

use std::alloc::Layout;
use std::hash::Hash;
use std::hash::Hasher;
use std::io::Read;
use std::io::Write;

use anyhow::Result;
use parking_lot::Mutex;
use serde::de::DeserializeOwned;
use serde::Serialize;

pub struct ShmStore<K, V> {
    /// An LRU cache of hashconsed values in front of the serialized shm heap.
    cache: Mutex<lru::LruCache<K, V>>,
    evictable: bool,
    compression: Compression,
    prefix: &'static str,
}

#[derive(Debug, Copy, Clone)]
pub enum Evictability {
    NonEvictable,
    Evictable,
}

#[derive(Debug, Copy, Clone)]
pub enum Compression {
    None,
    Lz4 { compression_level: u32 },
    Zstd { compression_level: i32 },
}

impl Default for Compression {
    fn default() -> Self {
        Self::Lz4 {
            compression_level: 1,
        }
    }
}

impl<K, V> ShmStore<K, V>
where
    K: Key + Copy + Hash + Eq + Send + Sync + 'static,
    V: Clone + Serialize + DeserializeOwned + Send + Sync + 'static,
{
    pub fn new(prefix: &'static str, evictability: Evictability, compression: Compression) -> Self {
        Self {
            cache: Mutex::new(lru::LruCache::new(1000)),
            evictable: matches!(evictability, Evictability::Evictable),
            compression,
            prefix,
        }
    }

    fn hash_key(&self, key: K) -> u64 {
        let mut hasher = hash::Hasher::default();
        self.prefix.hash(&mut hasher);
        key.hash_key(&mut hasher);
        hasher.finish()
    }
}

impl<K, V> datastore::Store<K, V> for ShmStore<K, V>
where
    K: Key + Copy + Hash + Eq + Send + Sync + 'static,
    V: Clone + Serialize + DeserializeOwned + Send + Sync + 'static,
{
    fn get(&self, key: K) -> Result<Option<V>> {
        if let Some(val) = self.cache.lock().get(&key) {
            return Ok(Some(val.clone()));
        }
        let hash = self.hash_key(key);
        let val_opt: Option<V> = shmffi::with(|segment| {
            segment
                .table
                .get(&hash)
                .map(|heap_value| match self.compression {
                    Compression::None => deserialize(heap_value.as_slice()),
                    Compression::Lz4 { .. } => {
                        lz4_decompress_and_deserialize(heap_value.as_slice())
                    }
                    Compression::Zstd { .. } => {
                        zstd_decompress_and_deserialize(heap_value.as_slice())
                    }
                })
                .transpose()
        })?;
        if let Some(val) = &val_opt {
            self.cache.lock().put(key, val.clone());
        }
        Ok(val_opt)
    }

    fn insert(&self, key: K, val: V) -> Result<()> {
        let blob = match self.compression {
            Compression::None => serialize(&val)?,
            Compression::Lz4 { compression_level } => {
                serialize_and_lz4_compress(&val, compression_level)?
            }
            Compression::Zstd { compression_level } => {
                serialize_and_zstd_compress(&val, compression_level)?
            }
        };
        self.cache.lock().put(key, val);
        let blob = ocaml_blob::SerializedValue::BStr(&blob);
        let _did_insert = shmffi::with(|segment| {
            segment.table.insert(
                self.hash_key(key),
                Some(Layout::from_size_align(blob.as_slice().len(), 1).unwrap()),
                self.evictable,
                |buffer| blob.to_heap_value_in(self.evictable, buffer),
            )
        });
        Ok(())
    }

    fn remove_batch(&self, keys: &mut dyn Iterator<Item = K>) -> Result<()> {
        let mut cache = self.cache.lock();
        for key in keys {
            let hash = self.hash_key(key);
            let _size = shmffi::with(|segment| {
                segment
                    .table
                    .inspect_and_remove(&hash, |value| value.unwrap().as_slice().len())
            });
            cache.pop(&key);
        }
        Ok(())
    }
}

fn serialize<T: Serialize>(val: &T) -> Result<Vec<u8>> {
    let mut serialized = Vec::new();
    bincode::serialize_into(&mut serialized, &intern::WithIntern(val))?;
    Ok(serialized)
}

fn deserialize<T: DeserializeOwned>(serialized: &[u8]) -> Result<T> {
    Ok(intern::WithIntern::strip(bincode::deserialize(serialized))?)
}

fn serialize_and_lz4_compress<T: Serialize>(val: &T, level: u32) -> Result<Vec<u8>> {
    let encoder = lz4::EncoderBuilder::new().level(level).build(vec![])?;
    let mut w = std::io::BufWriter::new(encoder);
    bincode::serialize_into(&mut w, &intern::WithIntern(val))?;
    w.flush()?;
    let encoder = w.into_inner().expect("into_inner returned Err after flush");
    let (compressed, result) = encoder.finish();
    result?;
    Ok(compressed)
}

fn lz4_decompress_and_deserialize<R: Read, T: DeserializeOwned>(r: R) -> Result<T> {
    let r = lz4::Decoder::new(r)?;
    let mut r = std::io::BufReader::new(r);
    Ok(intern::WithIntern::strip(bincode::deserialize_from(
        &mut r,
    ))?)
}

fn serialize_and_zstd_compress<T: Serialize>(val: &T, level: i32) -> Result<Vec<u8>> {
    let mut compressed = vec![];
    let w = zstd::Encoder::new(&mut compressed, level)?.auto_finish();
    let mut w = std::io::BufWriter::new(w);
    bincode::serialize_into(&mut w, &intern::WithIntern(val))?;
    drop(w);
    Ok(compressed)
}

fn zstd_decompress_and_deserialize<R: Read, T: DeserializeOwned>(r: R) -> Result<T> {
    let r = zstd::Decoder::new(r)?;
    let mut r = std::io::BufReader::new(r);
    Ok(intern::WithIntern::strip(bincode::deserialize_from(
        &mut r,
    ))?)
}

impl<K, V> std::fmt::Debug for ShmStore<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ShmStore").finish()
    }
}

/// There seems to be a problem with using the impl of `Hash` for interned
/// symbols: since they're only 32-bit IDs, hashes based on them tend to
/// collide, which our shmrs library cannot tolerate. Instead, we use this
/// custom hashing trait and hash the entire string representation of the
/// symbol. We might want to revisit this later and see whether there's a way to
/// hash these less expensively.
pub trait Key {
    fn hash_key<H: Hasher>(&self, state: &mut H);
}

impl Key for pos::TypeName {
    fn hash_key<H: Hasher>(&self, state: &mut H) {
        self.as_str().hash(state);
    }
}
impl Key for pos::ModuleName {
    fn hash_key<H: Hasher>(&self, state: &mut H) {
        self.as_str().hash(state);
    }
}
impl Key for pos::FunName {
    fn hash_key<H: Hasher>(&self, state: &mut H) {
        self.as_str().hash(state);
    }
}
impl Key for pos::ConstName {
    fn hash_key<H: Hasher>(&self, state: &mut H) {
        self.as_str().hash(state);
    }
}

impl<T: AsRef<str>> Key for (pos::TypeName, T) {
    fn hash_key<H: Hasher>(&self, state: &mut H) {
        let type_name: &str = self.0.as_ref();
        type_name.hash(state);
        let member_name: &str = self.1.as_ref();
        member_name.hash(state);
    }
}

impl Key for pos::RelativePath {
    fn hash_key<H: Hasher>(&self, state: &mut H) {
        self.prefix().hash(state);
        self.suffix().as_bytes().hash(state);
    }
}

impl Key for hh24_types::ToplevelSymbolHash {
    fn hash_key<H: Hasher>(&self, state: &mut H) {
        self.hash(state);
    }
}

impl Key for hh24_types::ToplevelCanonSymbolHash {
    fn hash_key<H: Hasher>(&self, state: &mut H) {
        self.hash(state);
    }
}
