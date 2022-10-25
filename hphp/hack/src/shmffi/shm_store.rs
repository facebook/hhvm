// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(allocator_api)]

use std::alloc::Layout;
use std::borrow::Cow;
use std::hash::Hash;
use std::hash::Hasher;
use std::io::Read;
use std::io::Write;

use anyhow::Result;
use ocamlrep::ptr::UnsafeOcamlPtr;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use parking_lot::Mutex;
use serde::de::DeserializeOwned;
use serde::Serialize;

/// A `datastore::Store` which writes its values to sharedmem (via the `shmffi`
/// crate) as bincode-serialized values. Can be configured to compress the
/// bincode blobs using `Compression`.
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
    fn contains_key(&self, key: K) -> Result<bool> {
        if self.cache.lock().contains(&key) {
            return Ok(true);
        }
        Ok(shmffi::with(|segment| {
            segment.table.contains_key(&self.hash_key(key))
        }))
    }

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

    fn move_batch(&self, keys: &mut dyn Iterator<Item = (K, K)>) -> Result<()> {
        let mut cache = self.cache.lock();
        for (old_key, new_key) in keys {
            let old_hash = self.hash_key(old_key);
            let new_hash = self.hash_key(new_key);
            shmffi::with(|segment| {
                let (header, data) = segment.table.inspect_and_remove(&old_hash, |value| {
                    let value = value.unwrap();
                    (value.header, <Box<[u8]>>::from(value.as_slice()))
                });
                cache.pop(&old_key);
                segment.table.insert(
                    new_hash,
                    Some(Layout::from_size_align(data.len(), 1).unwrap()),
                    header.is_evictable(),
                    |buffer| {
                        buffer.copy_from_slice(&data);
                        ocaml_blob::HeapValue {
                            header,
                            data: std::ptr::NonNull::new(buffer.as_mut_ptr()).unwrap(),
                        }
                    },
                );
                // We choose not to `cache.put(new_key, ...)` here.
            });
        }
        Ok(())
    }

    fn remove_batch(&self, keys: &mut dyn Iterator<Item = K>) -> Result<()> {
        let mut cache = self.cache.lock();
        for key in keys {
            cache.pop(&key);

            let hash = self.hash_key(key);
            let contains = shmffi::with(|segment| segment.table.contains_key(&hash));
            if !contains {
                continue;
            }

            let _size = shmffi::with(|segment| {
                segment
                    .table
                    .inspect_and_remove(&hash, |value| value.unwrap().as_slice().len())
            });
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

/// A `datastore::Store` which writes its values to sharedmem (via the `shmffi`
/// crate) as OCaml-marshaled values. Can be configured to compress the
/// marshaled blobs using `Compression`.
pub struct OcamlShmStore<K, V> {
    /// An LRU cache of hashconsed values in front of the serialized shm heap.
    cache: Mutex<lru::LruCache<K, V>>,
    evictable: bool,
    compression: Compression,
    prefix: &'static str,
}

impl<K, V> OcamlShmStore<K, V>
where
    K: Key + Copy + Hash + Eq + Send + Sync + 'static,
    V: Clone + Send + Sync + 'static,
{
    pub fn new(prefix: &'static str, evictability: Evictability, compression: Compression) -> Self {
        Self {
            cache: Mutex::new(lru::LruCache::new(1000)),
            evictable: matches!(evictability, Evictability::Evictable),
            compression,
            prefix,
        }
    }

    /// # Safety
    ///
    /// Must be invoked on the main thread. Calls into the OCaml runtime and may
    /// trigger a GC, so no unrooted OCaml values may exist. The returned
    /// `UnsafeOcamlPtr` is unrooted and could be invalidated if the GC is
    /// triggered after this method returns.
    pub unsafe fn get_ocaml_value(&self, key: K) -> Option<UnsafeOcamlPtr> {
        shmffi::with(|segment| {
            segment.table.get(&self.hash_key(key)).map(|heap_value| {
                extern "C" {
                    fn caml_input_value_from_block(data: *const u8, size: usize) -> UnsafeOcamlPtr;
                }
                let bytes = self.decompress(heap_value.as_slice()).unwrap();
                caml_input_value_from_block(bytes.as_ptr(), bytes.len())
            })
        })
    }

    fn decompress<'a>(&self, bytes: &'a [u8]) -> Result<Cow<'a, [u8]>> {
        Ok(match self.compression {
            Compression::None => Cow::Borrowed(bytes),
            Compression::Lz4 { .. } => Cow::Owned(lz4_decompress(bytes)?),
            Compression::Zstd { .. } => Cow::Owned(zstd_decompress(bytes)?),
        })
    }

    fn hash_key(&self, key: K) -> u64 {
        let mut hasher = hash::Hasher::default();
        self.prefix.hash(&mut hasher);
        key.hash_key(&mut hasher);
        hasher.finish()
    }
}

impl<K, V> datastore::Store<K, V> for OcamlShmStore<K, V>
where
    K: Key + Copy + Hash + Eq + Send + Sync + 'static,
    V: ToOcamlRep + FromOcamlRep + Clone + Send + Sync + 'static,
{
    fn contains_key(&self, key: K) -> Result<bool> {
        if self.cache.lock().contains(&key) {
            return Ok(true);
        }
        Ok(shmffi::with(|segment| {
            segment.table.contains_key(&self.hash_key(key))
        }))
    }

    fn get(&self, key: K) -> Result<Option<V>> {
        if let Some(val) = self.cache.lock().get(&key) {
            return Ok(Some(val.clone()));
        }
        let hash = self.hash_key(key);
        let val_opt: Option<V> = shmffi::with(|segment| {
            segment
                .table
                .get(&hash)
                .map(|heap_value| -> Result<_> {
                    let bytes = self.decompress(heap_value.as_slice()).unwrap();
                    let arena = ocamlrep::Arena::new();
                    let value = unsafe { ocamlrep_marshal::input_value(&bytes, &arena) };
                    // SAFETY: we just allocated this value with an ocamlrep::Arena
                    let value = unsafe { ocamlrep::Arena::make_transparent(value) };
                    Ok(V::from_ocamlrep(value)?)
                })
                .transpose()
        })?;
        if let Some(val) = &val_opt {
            self.cache.lock().put(key, val.clone());
        }
        Ok(val_opt)
    }

    fn insert(&self, key: K, val: V) -> Result<()> {
        let arena = ocamlrep::Arena::new();
        let ocaml_val = arena.add_root(&val);
        let mut bytes = std::io::Cursor::new(Vec::with_capacity(4096));
        ocamlrep_marshal::output_value(
            &mut bytes,
            ocaml_val,
            ocamlrep_marshal::ExternFlags::empty(),
        )?;
        let bytes = bytes.into_inner();
        let bytes = match self.compression {
            Compression::None => bytes,
            Compression::Lz4 { compression_level } => lz4_compress(&bytes, compression_level)?,
            Compression::Zstd { compression_level } => zstd_compress(&bytes, compression_level)?,
        };
        self.cache.lock().put(key, val);
        let blob = ocaml_blob::SerializedValue::BStr(&bytes);
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

    fn move_batch(&self, keys: &mut dyn Iterator<Item = (K, K)>) -> Result<()> {
        let mut cache = self.cache.lock();
        for (old_key, new_key) in keys {
            let old_hash = self.hash_key(old_key);
            let new_hash = self.hash_key(new_key);
            shmffi::with(|segment| {
                let (header, data) = segment.table.inspect_and_remove(&old_hash, |value| {
                    let value = value.unwrap();
                    (value.header, <Box<[u8]>>::from(value.as_slice()))
                });
                cache.pop(&old_key);
                segment.table.insert(
                    new_hash,
                    Some(Layout::from_size_align(data.len(), 1).unwrap()),
                    header.is_evictable(),
                    |buffer| {
                        buffer.copy_from_slice(&data);
                        ocaml_blob::HeapValue {
                            header,
                            data: std::ptr::NonNull::new(buffer.as_mut_ptr()).unwrap(),
                        }
                    },
                );
                // We choose not to `cache.put(new_key, ...)` here.
            });
        }
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

fn lz4_compress(mut bytes: &[u8], level: u32) -> Result<Vec<u8>> {
    let mut encoder = lz4::EncoderBuilder::new().level(level).build(vec![])?;
    std::io::copy(&mut bytes, &mut encoder)?;
    let (compressed, result) = encoder.finish();
    result?;
    Ok(compressed)
}

fn lz4_decompress(compressed: &[u8]) -> Result<Vec<u8>> {
    let mut decompressed = vec![];
    let mut decoder = lz4::Decoder::new(compressed)?;
    std::io::copy(&mut decoder, &mut decompressed)?;
    Ok(decompressed)
}

fn zstd_compress(mut bytes: &[u8], level: i32) -> Result<Vec<u8>> {
    let mut compressed = vec![];
    zstd::stream::copy_encode(&mut bytes, &mut compressed, level)?;
    Ok(compressed)
}

fn zstd_decompress(mut compressed: &[u8]) -> Result<Vec<u8>> {
    let mut decompressed = vec![];
    zstd::stream::copy_decode(&mut compressed, &mut decompressed)?;
    Ok(decompressed)
}

impl<K, V> std::fmt::Debug for OcamlShmStore<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("OcamlShmStore").finish()
    }
}
