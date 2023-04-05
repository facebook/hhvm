// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![feature(allocator_api)]

use std::alloc::Layout;
use std::borrow::Borrow;
use std::borrow::Cow;
use std::hash::Hash;
use std::io::Read;
use std::io::Write;
use std::num::NonZeroUsize;

use anyhow::Result;
use md5::Digest;
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
            cache: Mutex::new(lru::LruCache::new(NonZeroUsize::new(1000).unwrap())),
            evictable: matches!(evictability, Evictability::Evictable),
            compression,
            prefix,
        }
    }

    fn hash_key(&self, key: K) -> u64 {
        let mut hasher = md5::Md5::new();
        hasher.update(self.prefix);
        key.hash_key(&mut hasher);
        // hh_shared just takes the first 64 bits of the 128-bit MD5 digest.
        u64::from_ne_bytes((&hasher.finalize()[0..8]).try_into().unwrap())
    }

    #[rustfmt::skip]
    fn log_serialize(&self, size_in_shm: usize) {
        let size_in_shm = size_in_shm as f64;
        // shmrs doesn't actually allow us to count the total including
        // header/padding/alignment. `shmffi` just reuses the `compressed`
        // number for this stat, so do the same for now.
        measure::sample((self.prefix, "total bytes including header and padding"), size_in_shm);
        measure::sample(("ALL bytes", "total bytes including header and padding"), size_in_shm);
        measure::sample((self.prefix, "bytes serialized into shared heap"), size_in_shm);
        measure::sample("ALL bytes serialized into shared heap", size_in_shm);
    }

    #[rustfmt::skip]
    fn log_deserialize(&self, compressed_size: usize) {
        measure::sample((self.prefix, "bytes deserialized from shared heap"), compressed_size as f64);
        measure::sample("ALL bytes deserialized from shared heap", compressed_size as f64);
    }

    #[rustfmt::skip]
    fn log_shmem_hit_rate(&self, is_hit: bool) {
        measure::sample((self.prefix, "shmem cache hit rate"), is_hit as u8 as f64);
        measure::sample("ALL shmem cache hit rate", is_hit as u8 as f64);
    }

    #[rustfmt::skip]
    fn log_cache_hit_rate(&self, is_hit: bool) {
        measure::sample((self.prefix, "rust cache hit rate"), is_hit as u8 as f64);
        measure::sample("ALL rust cache hit rate", is_hit as u8 as f64);
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
        let cache_val_opt = self.cache.lock().get(&key).cloned();
        self.log_cache_hit_rate(cache_val_opt.is_some());
        if cache_val_opt.is_some() {
            return Ok(cache_val_opt);
        }
        let hash = self.hash_key(key);
        let val_opt: Option<V> = shmffi::with(|segment| {
            segment
                .table
                .get(&hash)
                .map(|heap_value| {
                    let bytes = heap_value.as_slice();
                    self.log_deserialize(bytes.len());
                    match self.compression {
                        Compression::None => deserialize(bytes),
                        Compression::Lz4 { .. } => lz4_decompress_and_deserialize(bytes),
                        Compression::Zstd { .. } => zstd_decompress_and_deserialize(bytes),
                    }
                })
                .transpose()
        })?;
        if let Some(val) = &val_opt {
            self.cache.lock().put(key, val.clone());
        }
        self.log_shmem_hit_rate(val_opt.is_some());
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
        let compressed_size = blob.len();
        let blob = ocaml_blob::SerializedValue::BStr(&blob);
        let did_insert = shmffi::with(|segment| {
            segment.table.insert(
                self.hash_key(key),
                Some(Layout::from_size_align(blob.as_slice().len(), 1).unwrap()),
                self.evictable,
                |buffer| blob.to_heap_value_in(self.evictable, buffer),
            )
        });
        if did_insert {
            self.log_serialize(compressed_size);
        }
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
///
/// If an implementor of this trait also implements `Borrow<[u8]>`, its impl of
/// `hash_key` must behave the same as `impl Key for [u8]` (which just invokes
/// `state.write(self)`).
pub trait Key {
    fn hash_key<H: Digest>(&self, state: &mut H);
}

impl Key for [u8] {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        state.update(self);
    }
}
impl Key for pos::TypeName {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        state.update(self.as_bytes());
    }
}
impl Key for pos::ModuleName {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        state.update(self.as_bytes());
    }
}
impl Key for pos::FunName {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        state.update(self.as_bytes());
    }
}
impl Key for pos::ConstName {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        state.update(self.as_bytes());
    }
}

impl<T: AsRef<str>> Key for (pos::TypeName, T) {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        let type_name: &str = self.0.as_ref();
        state.update(type_name);
        let member_name: &str = self.1.as_ref();
        state.update(member_name);
    }
}

impl Key for pos::RelativePath {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        state.update([self.prefix() as u8]);
        state.update(self.suffix());
    }
}

impl Key for hh24_types::ToplevelSymbolHash {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        state.update(self.as_u64().to_ne_bytes())
    }
}

impl Key for hh24_types::ToplevelCanonSymbolHash {
    fn hash_key<H: Digest>(&self, state: &mut H) {
        state.update(self.as_u64().to_ne_bytes())
    }
}

extern "C" {
    fn hh_log_level() -> ocamlrep::Value<'static>;
}

fn shm_log_level() -> isize {
    // SAFETY: We rely on sharedmem having been initialized here.
    unsafe { hh_log_level() }.as_int().unwrap()
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
            cache: Mutex::new(lru::LruCache::new(NonZeroUsize::new(1000).unwrap())),
            evictable: matches!(evictability, Evictability::Evictable),
            compression,
            prefix,
        }
    }

    /// Fetch the value corresponding to the given key and deserialize it
    /// directly onto the OCaml heap.
    ///
    /// # Safety
    ///
    /// Must be invoked on the main thread. Calls into the OCaml runtime and may
    /// trigger a GC, so no unrooted OCaml values may exist. The returned
    /// `UnsafeOcamlPtr` is unrooted and could be invalidated if the GC is
    /// triggered after this method returns.
    pub unsafe fn get_ocaml(&self, key: K) -> Option<UnsafeOcamlPtr> {
        self.get_ocaml_by_hash(self.hash_key(&key))
    }

    /// Fetch the value corresponding to the given key (when the key type `K`
    /// can be represented with a byte string, and implements `Borrow<[u8]>`)
    /// and deserialize it directly onto the OCaml heap.
    ///
    /// # Safety
    ///
    /// Must be invoked on the main thread. Calls into the OCaml runtime and may
    /// trigger a GC, so no unrooted OCaml values may exist. The returned
    /// `UnsafeOcamlPtr` is unrooted and could be invalidated if the GC is
    /// triggered after this method returns.
    pub unsafe fn get_ocaml_by_byte_string(&self, key: &[u8]) -> Option<UnsafeOcamlPtr>
    where
        K: Borrow<[u8]>,
    {
        self.get_ocaml_by_hash(self.hash_key(key))
    }

    unsafe fn get_ocaml_by_hash(&self, hash: u64) -> Option<UnsafeOcamlPtr> {
        extern "C" {
            fn caml_input_value_from_block(data: *const u8, size: usize) -> UnsafeOcamlPtr;
        }
        let bytes_opt = shmffi::with(|segment| {
            segment.table.get(&hash).map(|heap_value| {
                self.decompress(heap_value.as_slice(), heap_value.header.uncompressed_size())
                    .unwrap()
                    .into_owned()
            })
        });
        let v = bytes_opt.map(|bytes| caml_input_value_from_block(bytes.as_ptr(), bytes.len()));
        self.log_shmem_hit_rate(v.is_some());
        v
    }

    fn decompress<'a>(&self, bytes: &'a [u8], uncompressed_size: usize) -> Result<Cow<'a, [u8]>> {
        self.log_deserialize(bytes.len());
        Ok(match self.compression {
            Compression::None => Cow::Borrowed(bytes),
            Compression::Lz4 { .. } => Cow::Owned(lz4_decompress(bytes, uncompressed_size)?),
            Compression::Zstd { .. } => Cow::Owned(zstd_decompress(bytes)?),
        })
    }

    fn hash_key<Q: ?Sized + Key>(&self, key: &Q) -> u64
    where
        K: Borrow<Q>,
    {
        let mut hasher = md5::Md5::new();
        hasher.update(self.prefix);
        key.hash_key(&mut hasher);
        // hh_shared just takes the first 64 bits of the 128-bit MD5 digest.
        u64::from_ne_bytes((&hasher.finalize()[0..8]).try_into().unwrap())
    }

    #[rustfmt::skip]
    fn log_serialize(&self, compressed: usize, original: usize) {
        if shm_log_level() < 1 {
            return;
        }
        let compressed = compressed as f64;
        let original = original as f64;
        let saved = original - compressed;
        let ratio = compressed / original;
        // shmrs doesn't actually allow us to count the total including
        // header/padding/alignment. `shmffi` just reuses the `compressed`
        // number for this stat, so do the same for now.
        measure::sample((self.prefix, "total bytes including header and padding"), compressed);
        measure::sample(("ALL bytes", "total bytes including header and padding"), compressed);
        measure::sample((self.prefix, "bytes serialized into shared heap"), compressed);
        measure::sample("ALL bytes serialized into shared heap", compressed);
        measure::sample((self.prefix, "bytes saved in shared heap due to compression"), saved);
        measure::sample("ALL bytes saved in shared heap due to compression", saved);
        measure::sample((self.prefix, "shared heap compression ratio"), ratio);
        measure::sample("ALL bytes shared heap compression ratio", ratio);
    }

    #[rustfmt::skip]
    fn log_deserialize(&self, compressed_size: usize) {
        if shm_log_level() < 1 {
            return;
        }
        measure::sample((self.prefix, "bytes deserialized from shared heap"), compressed_size as f64);
        measure::sample("ALL bytes deserialized from shared heap", compressed_size as f64);
    }

    #[rustfmt::skip]
    fn log_shmem_hit_rate(&self, is_hit: bool) {
        if shm_log_level() < 1 {
            return;
        }
        measure::sample((self.prefix, "shmem cache hit rate"), is_hit as u8 as f64);
        measure::sample("ALL shmem cache hit rate", is_hit as u8 as f64);
    }

    #[rustfmt::skip]
    fn log_cache_hit_rate(&self, is_hit: bool) {
        if shm_log_level() < 1 {
            return;
        }
        measure::sample((self.prefix, "rust cache hit rate"), is_hit as u8 as f64);
        measure::sample("ALL rust cache hit rate", is_hit as u8 as f64);
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
            segment.table.contains_key(&self.hash_key(&key))
        }))
    }

    fn get(&self, key: K) -> Result<Option<V>> {
        let cache_val_opt = self.cache.lock().get(&key).cloned();
        self.log_cache_hit_rate(cache_val_opt.is_some());
        if cache_val_opt.is_some() {
            return Ok(cache_val_opt);
        }
        let bytes_opt = shmffi::with(|segment| {
            segment
                .table
                .get(&self.hash_key(&key))
                .map(|heap_value| -> Result<_> {
                    Ok(self
                        .decompress(heap_value.as_slice(), heap_value.header.uncompressed_size())?
                        .into_owned())
                })
                .transpose()
        })?;
        let val_opt: Option<V> = bytes_opt
            .map(|bytes| -> Result<_> {
                let arena = ocamlrep::Arena::new();
                let value = unsafe { ocamlrep_marshal::input_value(&bytes, &arena) };
                Ok(V::from_ocamlrep(value)?)
            })
            .transpose()?;
        if let Some(val) = &val_opt {
            self.cache.lock().put(key, val.clone());
        }
        self.log_shmem_hit_rate(val_opt.is_some());
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
        let uncompressed_size = bytes.len();
        let bytes = match self.compression {
            Compression::None => bytes,
            Compression::Lz4 { .. } => lz4_compress(&bytes)?,
            Compression::Zstd { compression_level } => zstd_compress(&bytes, compression_level)?,
        };
        let compressed_size = bytes.len();
        self.cache.lock().put(key, val);
        let did_insert = shmffi::with(|segment| {
            segment.table.insert(
                self.hash_key(&key),
                Some(Layout::from_size_align(bytes.len(), 1).unwrap()),
                self.evictable,
                |buffer| {
                    buffer.copy_from_slice(&bytes);
                    let header = ocaml_blob::HeapValueHeaderFields {
                        buffer_size: bytes.len(),
                        uncompressed_size,
                        is_serialized: true,
                        is_evictable: self.evictable,
                    };
                    ocaml_blob::HeapValue {
                        header: header.into(),
                        data: std::ptr::NonNull::from(buffer).cast(),
                    }
                },
            )
        });
        if did_insert {
            self.log_serialize(compressed_size, uncompressed_size);
        }
        Ok(())
    }

    fn move_batch(&self, keys: &mut dyn Iterator<Item = (K, K)>) -> Result<()> {
        let mut cache = self.cache.lock();
        for (old_key, new_key) in keys {
            let old_hash = self.hash_key(&old_key);
            let new_hash = self.hash_key(&new_key);
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

            let hash = self.hash_key(&key);
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

fn lz4_compress(bytes: &[u8]) -> Result<Vec<u8>> {
    Ok(lz4::block::compress(bytes, None, false)?)
}

fn lz4_decompress(compressed: &[u8], uncompressed_size: usize) -> Result<Vec<u8>> {
    Ok(lz4::block::decompress(
        compressed,
        Some(uncompressed_size.try_into().unwrap()),
    )?)
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
