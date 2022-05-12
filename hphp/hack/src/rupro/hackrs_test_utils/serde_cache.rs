// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use dashmap::DashMap;
use serde::{de::DeserializeOwned, Serialize};
use std::cmp::Eq;
use std::fmt::Debug;
use std::hash::Hash;

pub struct SerializingCache<K: Hash + Eq, V: Serialize + DeserializeOwned> {
    /// A non-evicting cache for serialized values.
    cache: DashMap<K, Box<[u8]>>,
    /// An LRU cache of hashconsed values, in front of the non-evicting
    /// serialized cache.
    evicting_cache: moka::sync::SegmentedCache<K, V>,
    compression: Compression,
}

#[derive(Copy, Clone, Debug)]
pub enum Compression {
    None,
    Zstd,
    Lz4,
}

#[derive(Copy, Clone, Debug)]
pub enum CacheOpts {
    Unserialized,
    Serialized(Compression),
}

impl<K, V> Default for SerializingCache<K, V>
where
    K: Copy + Hash + Eq + Send + Sync + 'static,
    V: Clone + Serialize + DeserializeOwned + Send + Sync + 'static,
{
    fn default() -> Self {
        Self {
            cache: Default::default(),
            evicting_cache: moka::sync::SegmentedCache::new(1024, 32),
            compression: Default::default(),
        }
    }
}

impl Default for Compression {
    fn default() -> Self {
        Self::Zstd
    }
}

impl<K, V> SerializingCache<K, V>
where
    K: Copy + Hash + Eq + Send + Sync + 'static,
    V: Clone + Serialize + DeserializeOwned + Send + Sync + 'static,
{
    pub fn new() -> Self {
        Default::default()
    }

    pub fn with_compression(compression: Compression) -> Self {
        Self {
            compression,
            ..Default::default()
        }
    }
}

impl<K, V> hackrs::cache::Cache<K, V> for SerializingCache<K, V>
where
    K: Copy + Hash + Eq + Send + Sync + 'static,
    V: Clone + Serialize + DeserializeOwned + Send + Sync + 'static,
{
    fn get(&self, key: K) -> Option<V> {
        if let val @ Some(..) = self.evicting_cache.get(&key) {
            return val;
        }
        let val_opt: Option<V> = self.cache.get(&key).map(|val| match self.compression {
            Compression::None => deserialize(&val),
            Compression::Zstd => {
                let serialized = zstd_decompress(&val);
                deserialize(&serialized)
            }
            Compression::Lz4 => {
                let serialized = lz4_decompress(&val);
                deserialize(&serialized)
            }
        });
        val_opt.map(|val| self.evicting_cache.get_with(key, || val))
    }

    fn insert(&self, key: K, val: V) {
        let serialized = serialize(&val);
        self.evicting_cache.insert(key, val);
        let compressed = match self.compression {
            Compression::None => serialized,
            Compression::Zstd => zstd_compress(&serialized),
            Compression::Lz4 => lz4_compress(&serialized),
        };
        self.cache.insert(key, compressed.into_boxed_slice());
    }
}

fn serialize<T: Serialize>(val: &T) -> Vec<u8> {
    let mut serialized = Vec::new();
    bincode::serialize_into(&mut serialized, &intern::WithIntern(val)).unwrap();
    serialized
}

fn deserialize<T: DeserializeOwned>(serialized: &[u8]) -> T {
    intern::WithIntern::strip(bincode::deserialize(serialized)).unwrap()
}

fn zstd_compress(mut bytes: &[u8]) -> Vec<u8> {
    let mut compressed = vec![];
    zstd::stream::copy_encode(&mut bytes, &mut compressed, 0).unwrap();
    compressed
}

fn zstd_decompress(mut compressed: &[u8]) -> Vec<u8> {
    let mut decompressed = vec![];
    zstd::stream::copy_decode(&mut compressed, &mut decompressed).unwrap();
    decompressed
}

fn lz4_compress(mut bytes: &[u8]) -> Vec<u8> {
    let mut encoder = lz4::EncoderBuilder::new().level(1).build(vec![]).unwrap();
    std::io::copy(&mut bytes, &mut encoder).unwrap();
    let (compressed, result) = encoder.finish();
    result.unwrap();
    compressed
}

fn lz4_decompress(compressed: &[u8]) -> Vec<u8> {
    let mut decompressed = vec![];
    let mut decoder = lz4::Decoder::new(compressed).unwrap();
    std::io::copy(&mut decoder, &mut decompressed).unwrap();
    decompressed
}

impl<K, V> Debug for SerializingCache<K, V>
where
    K: Hash + Eq,
    V: Serialize + DeserializeOwned,
{
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("SerializingCache").finish()
    }
}

impl std::str::FromStr for Compression {
    type Err = &'static str;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "none" => Ok(Self::None),
            "zstd" => Ok(Self::Zstd),
            "lz4" => Ok(Self::Lz4),
            _ => Err("compression must be one of 'none', 'zstd', 'lz4'"),
        }
    }
}

impl std::fmt::Display for Compression {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::None => write!(f, "none"),
            Self::Zstd => write!(f, "zstd"),
            Self::Lz4 => write!(f, "lz4"),
        }
    }
}
