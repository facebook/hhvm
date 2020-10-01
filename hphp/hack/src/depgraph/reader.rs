// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::byteutils;
pub use crate::dep::Dep;

use std::convert::TryInto;
use std::ops::Deref;

use im_rc::OrdSet;

/// Use a `DepGraphOpener` to initialize a dependency graph from a file.
///
/// # Example
///
/// ```
/// let opener = DepGraphOpener::from_path("/tmp/").unwrap();
/// let depgraph = opener.open().unwrap();
/// ```
pub struct DepGraphOpener {
    mmap: memmap::Mmap,
}

impl DepGraphOpener {
    /// Create a dependency graph opener given an open file handle.
    ///
    /// The file handle can be safely closed afterwards.
    pub fn new(file: &std::fs::File) -> std::io::Result<Self> {
        // Safety: we rely on the memmap library to provide safety.
        let mmap = unsafe { memmap::Mmap::map(file) }?;
        Ok(Self { mmap })
    }

    /// Create a dependency graph opener given a file path.
    pub fn from_path<P: AsRef<std::path::Path>>(path: P) -> std::io::Result<Self> {
        let f = std::fs::OpenOptions::new().read(true).open(path)?;
        Self::new(&f)
    }

    /// Return the underlying memory map.
    pub fn mmap(&self) -> &memmap::Mmap {
        &self.mmap
    }

    /// Open the dependency graph, or return an error description.
    pub fn open(&self) -> Result<DepGraph, String> {
        DepGraph::from_mmap(&self.mmap)
    }
}

/// An open dependency graph.
///
/// The lifetime parameter represents the lifetime of the underlying
/// raw byte array.
pub struct DepGraph<'bytes> {
    data: &'bytes [u8],

    indexer: Indexer<'bytes>,
    lookup_table: LookupTable<'bytes>,
}

impl<'bytes> DepGraph<'bytes> {
    /// Initialize a dependency graph using the byte array
    /// from a memory map.
    pub fn from_mmap(mmap: &'bytes memmap::Mmap) -> Result<Self, String> {
        let bytes = mmap.deref();
        Self::new(bytes)
    }

    /// Initialize a dependency graph using the given byte array.
    pub fn new(data: &'bytes [u8]) -> Result<Self, String> {
        let header = Header::new(data)?;

        let s = Self {
            data,

            indexer: header.indexer,
            lookup_table: header.lookup_table,
        };
        Ok(s)
    }

    /// Make sure the database is not corrupt.
    ///
    /// If you got this far, the indexer and lookup table were
    /// successfully initialized. This function checks whether
    /// all hash lists can be properly read from disk.
    pub fn validate_hash_lists(&self) -> Result<(), String> {
        let len: usize = self.indexer.len();
        for index in 0..len {
            match self.lookup_table.get(index as u32) {
                Some(list_offset) => {
                    let data = byteutils::subslice(
                        self.data,
                        list_offset as usize..,
                        "hash list data during validation",
                    )?;
                    let _: HashList = HashList::new(data)?;
                }
                None => {}
            }
        }
        Ok(())
    }

    /// Query the hash list for a given hash.
    ///
    /// Returns `None` if there is no hash list related to the hash.
    ///
    /// # Panics
    ///
    /// Panics if the file is corrupt. Use `validate_hash_lists` when
    /// initializing the reader to avoid these panics.
    pub fn hash_list_for(&self, hash: Dep) -> Option<HashList<'bytes>> {
        let index = self.indexer.find(hash.into())?;
        self.hash_list_for_index(index)
    }

    /// Query the hash list for a given hash index.
    ///
    /// Returns `None` if there is no hash list related to the hash.
    ///
    fn hash_list_for_index(&self, index: u32) -> Option<HashList<'bytes>> {
        let list_offset = self.lookup_table.get(index)?;
        Some(HashList::new(&self.data[list_offset as usize..]).unwrap())
    }

    /// Return `true` iff the given hash list contains the index for the given hash.
    pub fn hash_list_contains(&self, hash_list: HashList<'bytes>, hash: Dep) -> bool {
        if let Some(index) = self.indexer.find(hash.into()) {
            hash_list.has_index(index)
        } else {
            false
        }
    }

    /// Return an iterator over all hashes in a hash list.
    pub fn hash_list_hashes(&self, hash_list: HashList<'bytes>) -> HashListIter<'bytes> {
        HashListIter {
            indexer: self.indexer,
            current: 0,
            indices: hash_list.indices,
        }
    }

    /// Add the direct typing dependencies for one dependency.
    pub fn add_typing_deps_for_dep(&self, acc: &mut OrdSet<Dep>, dep: Dep) {
        if let Some(dept_hash_list) = self.hash_list_for(dep) {
            for dept in self.hash_list_hashes(dept_hash_list) {
                acc.insert(dept);
            }
        }
    }

    /// Query the direct typing dependencies for the given set of dependencies.
    pub fn query_typing_deps_multi(&self, deps: &OrdSet<Dep>) -> OrdSet<Dep> {
        let mut acc = deps.clone();
        for dep in deps {
            self.add_typing_deps_for_dep(&mut acc, *dep);
        }
        acc
    }
}

/// The header of the structure.
///
/// Contains the offset to the the indexer and the lookup table.
///
/// Memory layout:
///
/// ```txt
///           32 bits
///  +-----------------------+
///  |    indexer offset     |
///  +-----------------------+
///  |  lookup table offset  |
///  +-----------------------+
/// ```
struct Header<'bytes> {
    indexer: Indexer<'bytes>,
    lookup_table: LookupTable<'bytes>,
}

impl<'bytes> Header<'bytes> {
    fn new(data: &'bytes [u8]) -> Result<Self, String> {
        if data.len() < 4 * 2 {
            return Err("not enough bytes to read header".to_string());
        }

        let indexer_offset = byteutils::read_u32_ne(data);
        let lookup_table_offset = byteutils::read_u32_ne(&data[4..]);

        let indexer_offset: usize = indexer_offset.try_into().unwrap();
        let lookup_table_offset: usize = lookup_table_offset.try_into().unwrap();

        let indexer_bytes = byteutils::subslice(data, indexer_offset.., "indexer_bytes")?;
        let lookup_table_bytes =
            byteutils::subslice(data, lookup_table_offset.., "lookup_table_bytes")?;

        let indexer = Indexer::new(indexer_bytes)?;
        let lookup_table = LookupTable::new(lookup_table_bytes, indexer.len())?;

        Ok(Header {
            indexer,
            lookup_table,
        })
    }
}

/// The indexer table.
///
/// The indexer table maps a hash to an index.
///
/// Memory layout:
///
/// ```txt
///     64 bits
///  +===========+
///  |  length   |
///  +===========+
///  |   hash1   |
///  +-----------+
///  |   hash2   |
///  +-----------+
///  |    ...    |
///  +===========+
/// ```
#[derive(Clone, Copy)]
struct Indexer<'bytes> {
    /// All hashes.
    hashes: &'bytes [u64],
}

impl<'bytes> Indexer<'bytes> {
    fn new(data: &'bytes [u8]) -> Result<Self, String> {
        if data.len() < 8 * 2 {
            return Err("not enough bytes to read indexer".to_string());
        }

        // Read in length
        let length = byteutils::read_u64_ne(data);
        if length == 0 {
            return Err("indexer: length must be >0".to_string());
        }
        if length > (1 << 32) - 1 {
            return Err("indexer: length is too big".to_string());
        }

        // Read in u64 array
        let length: usize = length as usize;
        let indexer_data = byteutils::subslice(data, 8.., "indexer_data")?;
        let indexer_data = byteutils::as_u64_slice(indexer_data)
            .ok_or_else(|| "indexer: data is not properly aligned".to_string())?;
        if indexer_data.len() < length {
            return Err("indexer: not enough hashes".to_string());
        }

        let hashes = &indexer_data[..length];
        Ok(Indexer { hashes })
    }

    /// The number if hashes in the indexer
    #[inline]
    fn len(self) -> usize {
        self.hashes.len()
    }

    /// Binary search the indexer to find the index of a hash.
    #[inline]
    fn find(self, hash: u64) -> Option<u32> {
        if let Ok(index) = self.hashes.binary_search(&hash) {
            Some(index as u32)
        } else {
            None
        }
    }

    /// Get the hash belonging to the given index.
    #[inline]
    fn get(self, index: u32) -> Option<u64> {
        self.hashes.get(index as usize).copied()
    }
}

/// The actual lookup table.
///
/// Currently this is a list of pointers to dependent-lists.
///
/// To index in a lookup table with a hash, you should first find
/// the hashes index using the indexer.
///
/// Memory layout:
///
/// ```txt
///           32 bits
///  +========================+
///  |   pointer for hash 1   |
///  +------------------------+
///  |   pointer for hash 2   |
///  +------------------------+
///  |           ...          |
///  +========================+
/// ```
#[derive(Clone, Copy)]
struct LookupTable<'bytes> {
    /// All hash list offsets in the table.
    hash_list_offsets: &'bytes [u32],
}

impl<'bytes> LookupTable<'bytes> {
    fn new(data: &'bytes [u8], len: usize) -> Result<Self, String> {
        // Read in u32 array
        let hash_list_offsets = byteutils::as_u32_slice(data)
            .ok_or_else(|| "lookup table: data is not properly aligned".to_string())?;
        if hash_list_offsets.len() < len {
            return Err("lookup table: not enough pointers".to_string());
        }

        let hash_list_offsets = &hash_list_offsets[..len];
        Ok(LookupTable { hash_list_offsets })
    }

    #[inline]
    fn get(self, index: u32) -> Option<u32> {
        let offset = self.hash_list_offsets.get(index as usize).copied()?;
        if offset == 0 { None } else { Some(offset) }
    }
}

/// A pointer to a list of hashes.
///
/// This data structure is read lazily.
///
/// Memory layout:
///
///
/// ```txt
///           32 bits
///  +========================+
///  |        length          |
///  +========================+
///  |     index of hash 1    |
///  +------------------------+
///  |     index of hash 2    |
///  +------------------------+
///  |           ...          |
///  +========================+
/// ```
#[derive(Clone, Copy)]
pub struct HashList<'bytes> {
    indices: &'bytes [u32],
}

#[allow(clippy::len_without_is_empty)]
impl<'bytes> HashList<'bytes> {
    fn new(data: &'bytes [u8]) -> Result<Self, String> {
        let data = byteutils::as_u32_slice(data)
            .ok_or_else(|| "hash list: not properly aligned".to_string())?;

        let len: u32 = *data
            .get(0)
            .ok_or_else(|| "hash list: couldn't read length".to_string())?;
        let len: usize = len as usize;

        let indices = byteutils::subslice(data, 1.., "hash list.data")?;
        if indices.len() < len as usize {
            return Err("hash list: not enough indices".to_string());
        }
        let indices = &indices[..len as usize];
        Ok(HashList { indices })
    }

    #[inline]
    pub fn len(self) -> u32 {
        self.indices.len() as u32
    }

    #[inline]
    fn has_index(self, index: u32) -> bool {
        self.indices.binary_search(&index).is_ok()
    }
}

/// Hash list iterator.
///
/// Iterates over the hashes in a hash list.
pub struct HashListIter<'bytes> {
    indexer: Indexer<'bytes>,
    current: usize,
    indices: &'bytes [u32],
}

impl<'a> Iterator for HashListIter<'a> {
    type Item = Dep;

    fn next(&mut self) -> Option<Self::Item> {
        if self.current >= self.indices.len() {
            None
        } else {
            let index = self.indices[self.current];
            let hash = self.indexer.get(index);
            self.current += 1;
            hash.map(Dep::new)
        }
    }
}
