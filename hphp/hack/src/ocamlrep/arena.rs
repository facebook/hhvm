// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::max;
use std::sync::atomic::{AtomicUsize, Ordering};

use crate::{block::Header, OcamlRep, Value};

struct Chunk<'a> {
    data: Box<[Value<'a>]>,
    index: usize,

    /// Pointer to the prev arena segment.
    prev: Option<Box<Chunk<'a>>>,
}

impl<'a> Chunk<'a> {
    fn with_capacity(capacity: usize) -> Self {
        Self {
            index: 0,
            data: vec![Value::int(0); capacity].into_boxed_slice(),
            prev: None,
        }
    }

    fn capacity(&self) -> usize {
        self.data.len()
    }

    fn can_fit(&self, requested_size: usize) -> bool {
        self.index + requested_size <= self.data.len()
    }

    #[inline]
    pub fn alloc(&mut self, requested_size: usize) -> *mut Value<'a> {
        let previous_index = self.index;
        self.index += requested_size;
        unsafe { self.data.as_mut_ptr().add(previous_index) }
    }
}

// The generation number is used solely to identify which arena a cached value
// belongs to in `RcOc`.
//
// We use usize::max_value() / 2 here to avoid colliding with ocamlpool
// generation numbers. This generation trick isn't sound with the use of
// multiple generation counters, but this mitigation should make it extremely
// difficult to mix up values allocated with ocamlpool and Arena in practice
// (one would have to serialize the same value with both, and only after
// increasing the ocamlpool generation by an absurd amount).
//
// If we add a third kind of allocator, we might want to rethink this strategy.
static NEXT_GENERATION: AtomicUsize = AtomicUsize::new(usize::max_value() / 2);

pub struct Arena<'a> {
    generation: usize,
    current_chunk: Chunk<'a>,
}

impl<'a> Arena<'a> {
    /// Create a new Arena with 4KB of capacity preallocated.
    pub fn new() -> Self {
        Self::with_capacity(1024 * 4)
    }

    /// Create a new Arena with `capacity_in_bytes` preallocated.
    pub fn with_capacity(capacity_in_bytes: usize) -> Self {
        let generation = NEXT_GENERATION.fetch_add(1, Ordering::SeqCst);
        let capacity_in_words = max(2, capacity_in_bytes / std::mem::size_of::<Value<'_>>());
        Self {
            generation,
            current_chunk: Chunk::with_capacity(capacity_in_words),
        }
    }

    pub fn generation(&self) -> usize {
        self.generation
    }

    #[inline]
    fn alloc(&mut self, requested_size: usize) -> *mut Value<'a> {
        if !self.current_chunk.can_fit(requested_size) {
            let prev_chunk_capacity = self.current_chunk.capacity();
            let prev_chunk = std::mem::replace(
                &mut self.current_chunk,
                Chunk::with_capacity(max(requested_size * 2, prev_chunk_capacity)),
            );
            self.current_chunk.prev = Some(Box::new(prev_chunk));
        }
        self.current_chunk.alloc(requested_size)
    }

    #[inline]
    pub fn block_with_size_and_tag(&mut self, size: usize, tag: u8) -> *mut Value<'a> {
        assert!(size > 0);
        let block = self.alloc(size + 1);
        let header = Header::new(size, tag);
        unsafe {
            *block = Value::from_bits(header.to_bits());
            block.add(1)
        }
    }

    #[inline(always)]
    pub fn block_with_size(&mut self, size: usize) -> *mut Value<'a> {
        self.block_with_size_and_tag(size, 0)
    }

    /// This method is marked unsafe because there is no bounds checking for
    /// this write. Take care that the index is less than the size of the block.
    #[inline(always)]
    pub unsafe fn set_field(block: *mut Value<'a>, index: usize, value: Value<'a>) {
        *block.add(index) = value;
    }

    #[inline(always)]
    pub fn add<T: OcamlRep>(&mut self, value: &T) -> Value<'a> {
        value.to_ocamlrep(self)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::time::Instant;

    #[test]
    fn test_alloc_block_of_three_fields() {
        let mut arena = Arena::with_capacity(1000);

        let block = unsafe {
            let block = arena.block_with_size(3);
            Arena::set_field(block, 0, Value::int(1));
            Arena::set_field(block, 1, Value::int(2));
            Arena::set_field(block, 2, Value::int(3));
            Value::from_ptr(block).as_block().unwrap()
        };

        assert_eq!(block.size(), 3);
        assert_eq!(block[0].as_int().unwrap(), 1);
        assert_eq!(block[1].as_int().unwrap(), 2);
        assert_eq!(block[2].as_int().unwrap(), 3);
    }

    #[test]
    fn test_large_allocs() {
        let mut arena = Arena::with_capacity(1000);

        let mut alloc_block = |size| {
            unsafe { Value::from_ptr(arena.block_with_size(size)) }
                .as_block()
                .unwrap()
        };

        let max = alloc_block(1000);
        assert_eq!(max.size(), 1000);

        let two_thousand = alloc_block(2000);
        assert_eq!(two_thousand.size(), 2000);

        let four_thousand = alloc_block(4000);
        assert_eq!(four_thousand.size(), 4000);
    }

    #[test]
    fn perf_test() {
        let mut arena = Arena::with_capacity(10_000);

        let mut alloc_block = |size| {
            unsafe { Value::from_ptr(arena.block_with_size(size)) }
                .as_block()
                .unwrap()
        };

        println!("Benchmarks for allocating [1] 200,000 times");
        let now = Instant::now();
        for _ in 0..200_000 {
            vec![0; 1].into_boxed_slice();
        }
        println!("Alloc: {:?}", now.elapsed());

        let now = Instant::now();
        for _ in 0..200_000 {
            alloc_block(1);
        }
        println!("Arena: {:?}", now.elapsed());

        println!("Benchmarks for allocating [5] 200,000 times");
        let now = Instant::now();
        for _ in 0..200_000 {
            vec![0; 5].into_boxed_slice();
        }
        println!("Alloc: {:?}", now.elapsed());

        let now = Instant::now();
        for _ in 0..200_000 {
            alloc_block(5);
        }
        println!("Arena: {:?}", now.elapsed());

        println!("Benchmarks for allocating [10] 200,000 times");
        let now = Instant::now();
        for _ in 0..200_000 {
            vec![0; 10].into_boxed_slice();
        }
        println!("Alloc: {:?}", now.elapsed());

        let now = Instant::now();
        for _ in 0..200_000 {
            alloc_block(10);
        }
        println!("Arena: {:?}", now.elapsed());
    }
}
