// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::{Cell, RefCell};
use std::cmp::max;
use std::sync::atomic::{AtomicUsize, Ordering};

use crate::{block::Header, Allocator, BlockBuilder, OcamlRep, Value};

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

/// An [`Allocator`](trait.Allocator.html) which builds values in Rust-managed
/// memory. The memory is freed when the Arena is dropped.
pub struct Arena<'a> {
    generation: Cell<usize>,
    current_chunk: RefCell<Chunk<'a>>,
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
            generation: Cell::new(generation),
            current_chunk: RefCell::new(Chunk::with_capacity(capacity_in_words)),
        }
    }

    #[inline]
    fn alloc(&self, requested_size: usize) -> &mut [Value<'a>] {
        if !self.current_chunk.borrow().can_fit(requested_size) {
            let prev_chunk_capacity = self.current_chunk.borrow().capacity();
            let prev_chunk = self.current_chunk.replace(Chunk::with_capacity(max(
                requested_size * 2,
                prev_chunk_capacity,
            )));
            self.current_chunk.borrow_mut().prev = Some(Box::new(prev_chunk));
        }
        let ptr = self.current_chunk.borrow_mut().alloc(requested_size);
        // Transmute away the lifetime. We want to simultaneously hand out
        // several mutable references to distinct allocated blocks within our
        // chunks. Blocks are non-overlapping, so this won't allow aliasing. We
        // need to ensure that the slices don't outlive the Arena, though.
        unsafe { std::slice::from_raw_parts_mut(ptr, requested_size) }
    }

    #[inline(always)]
    pub fn add<T: OcamlRep>(&self, value: &T) -> Value<'a> {
        value.to_ocamlrep(self)
    }
}

impl<'a> Allocator<'a> for Arena<'a> {
    #[inline(always)]
    fn generation(&self) -> usize {
        self.generation.get()
    }

    fn block_with_size_and_tag<'b>(&'b self, size: usize, tag: u8) -> BlockBuilder<'a, 'b> {
        let block = self.alloc(size + 1);
        let header = Header::new(size, tag);
        // Safety: We need to make sure that the Header written to index 0 of
        // this slice is never observed as a Value. We guarantee that by not
        // exposing raw Chunk memory--only allocated Values.
        block[0] = unsafe { Value::from_bits(header.to_bits()) };
        BlockBuilder::new(&mut block[1..])
    }

    #[inline(always)]
    fn set_field<'b>(block: &mut BlockBuilder<'a, 'b>, index: usize, value: Value<'a>) {
        block.0[index] = value;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::time::Instant;

    #[test]
    fn test_alloc_block_of_three_fields() {
        let arena = Arena::with_capacity(1000);

        let mut block = arena.block_with_size(3);
        Arena::set_field(&mut block, 0, Value::int(1));
        Arena::set_field(&mut block, 1, Value::int(2));
        Arena::set_field(&mut block, 2, Value::int(3));
        let block = block.build().as_block().unwrap();

        assert_eq!(block.size(), 3);
        assert_eq!(block[0].as_int().unwrap(), 1);
        assert_eq!(block[1].as_int().unwrap(), 2);
        assert_eq!(block[2].as_int().unwrap(), 3);
    }

    #[test]
    fn test_large_allocs() {
        let arena = Arena::with_capacity(1000);

        let alloc_block = |size| arena.block_with_size(size).build().as_block().unwrap();

        let max = alloc_block(1000);
        assert_eq!(max.size(), 1000);

        let two_thousand = alloc_block(2000);
        assert_eq!(two_thousand.size(), 2000);

        let four_thousand = alloc_block(4000);
        assert_eq!(four_thousand.size(), 4000);
    }

    #[test]
    fn perf_test() {
        let arena = Arena::with_capacity(10_000);

        let alloc_block = |size| arena.block_with_size(size).build().as_block().unwrap();

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
