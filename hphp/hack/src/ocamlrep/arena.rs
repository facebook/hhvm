// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::cmp::max;
use std::sync::atomic::{AtomicUsize, Ordering};

use crate::{
    block::Header, Allocator, BlockBuilder, MemoizationCache, OpaqueValue, ToOcamlRep, Value,
};

struct Chunk {
    data: Box<[OpaqueValue<'static>]>,
    index: usize,

    /// Pointer to the prev arena segment.
    prev: Option<Box<Chunk>>,
}

impl Chunk {
    fn with_capacity(capacity: usize) -> Self {
        Self {
            index: 0,
            data: vec![OpaqueValue::int(0); capacity].into_boxed_slice(),
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
    pub fn alloc(&mut self, requested_size: usize) -> &mut [OpaqueValue<'static>] {
        let previous_index = self.index;
        self.index += requested_size;
        &mut self.data[previous_index..self.index]
    }
}

// The generation number is used solely to identify which arena a cached value
// belongs to in `RcOc`.
//
// We use usize::max_value() / 2 here to avoid colliding with ocamlpool and
// SlabAllocator generation numbers (ocamlpool starts at 0, and SlabAllocator
// starts at usize::max_value() / 4). This generation trick isn't sound with the
// use of multiple generation counters, but this mitigation should make it
// extremely difficult to mix up values allocated with ocamlpool, Arena, and
// SlabAllocator in practice (one would have to serialize the same value with
// multiple Allocators, and only after increasing the generation of one by an
// absurd amount).
//
// If we add more allocators, we might want to rethink this strategy.
static NEXT_GENERATION: AtomicUsize = AtomicUsize::new(usize::max_value() / 2);

/// An [`Allocator`](trait.Allocator.html) which builds values in Rust-managed
/// memory. The memory is freed when the Arena is dropped.
pub struct Arena {
    generation: usize,
    current_chunk: RefCell<Chunk>,
    cache: MemoizationCache,
}

impl Arena {
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
            current_chunk: RefCell::new(Chunk::with_capacity(capacity_in_words)),
            cache: MemoizationCache::new(),
        }
    }

    #[inline]
    #[allow(clippy::mut_from_ref)]
    fn alloc<'a>(&'a self, requested_size: usize) -> &'a mut [OpaqueValue<'a>] {
        if !self.current_chunk.borrow().can_fit(requested_size) {
            let prev_chunk_capacity = self.current_chunk.borrow().capacity();
            let prev_chunk = self.current_chunk.replace(Chunk::with_capacity(max(
                requested_size * 2,
                prev_chunk_capacity,
            )));
            self.current_chunk.borrow_mut().prev = Some(Box::new(prev_chunk));
        }
        let mut chunk = self.current_chunk.borrow_mut();
        let slice = chunk.alloc(requested_size);
        // Transmute the 'static lifetime to 'a, to allow Values which point to
        // blocks allocated using this Arena to be stored in other such blocks.
        // The lifetime ensures that callers cannot allow such Values to outlive
        // the arena (and therefore outlive the block they point to). This
        // transmute violates the 'static lifetime in Chunk, so it is critical
        // for safety that we never expose a view of those Values to code
        // outside this module (using the type `Value<'static>`).
        // Also transmute the unnamed lifetime referring to the mutable borrow
        // of `chunk` to 'a. This allows callers to hold multiple mutable blocks
        // at once. This is safe because the blocks handed out by Chunk::alloc
        // are non-overlapping, so there is no aliasing.
        unsafe {
            std::mem::transmute::<&'_ mut [OpaqueValue<'static>], &'a mut [OpaqueValue<'a>]>(slice)
        }
    }

    /// # Safety
    ///
    /// Must be used only with values allocated by an `ocamlrep::Arena`.
    pub unsafe fn make_transparent(value: OpaqueValue<'_>) -> Value<'_> {
        Value::from_bits(value.to_bits())
    }

    #[inline(always)]
    pub fn add<T: ToOcamlRep + ?Sized>(&self, value: &T) -> Value<'_> {
        // SAFETY: We just allocated this value using `self`, which is an `Arena`.
        unsafe { Self::make_transparent(value.to_ocamlrep(self)) }
    }

    #[inline(always)]
    pub fn add_root<T: ToOcamlRep + ?Sized>(&self, value: &T) -> Value<'_> {
        // SAFETY: We just allocated this value using `self`, which is an `Arena`.
        unsafe { Self::make_transparent(Allocator::add_root(self, value)) }
    }
}

impl Allocator for Arena {
    #[inline(always)]
    fn generation(&self) -> usize {
        self.generation
    }

    fn block_with_size_and_tag(&self, size: usize, tag: u8) -> BlockBuilder<'_> {
        let block = self.alloc(size + 1);
        let header = Header::new(size, tag);
        // Safety: We need to make sure that the Header written to index 0 of
        // this slice is never observed as a Value. We guarantee that by not
        // exposing raw Chunk memory--only allocated Values.
        block[0] = unsafe { OpaqueValue::from_bits(header.to_bits()) };
        let slice = &mut block[1..];
        BlockBuilder::new(slice.as_ptr() as usize, slice.len())
    }

    #[inline(always)]
    fn set_field<'a>(&self, block: &mut BlockBuilder<'a>, index: usize, value: OpaqueValue<'a>) {
        unsafe { *self.block_ptr_mut(block).add(index) = value }
    }

    unsafe fn block_ptr_mut<'a>(&self, block: &mut BlockBuilder<'a>) -> *mut OpaqueValue<'a> {
        block.address() as *mut _
    }

    fn memoized<'a>(
        &'a self,
        ptr: usize,
        size: usize,
        f: impl FnOnce(&'a Self) -> OpaqueValue<'a>,
    ) -> OpaqueValue<'a> {
        let bits = self.cache.memoized(ptr, size, || f(self).to_bits());
        // SAFETY: The only memoized values in the cache are those computed in
        // the closure on the previous line. Since f returns OpaqueValue<'a>, any
        // cached bits must represent a valid OpaqueValue<'a>,
        unsafe { OpaqueValue::from_bits(bits) }
    }

    fn add_root<T: ToOcamlRep + ?Sized>(&self, value: &T) -> OpaqueValue<'_> {
        self.cache.with_cache(|| value.to_ocamlrep(self))
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
        arena.set_field(&mut block, 0, OpaqueValue::int(1));
        arena.set_field(&mut block, 1, OpaqueValue::int(2));
        arena.set_field(&mut block, 2, OpaqueValue::int(3));
        let block = block.build();
        // SAFETY: The block was allocated by an `Arena`.
        let block = unsafe { Arena::make_transparent(block) }
            .as_block()
            .unwrap();

        assert_eq!(block.size(), 3);
        assert_eq!(block[0].as_int().unwrap(), 1);
        assert_eq!(block[1].as_int().unwrap(), 2);
        assert_eq!(block[2].as_int().unwrap(), 3);
    }

    #[test]
    fn test_large_allocs() {
        let arena = Arena::with_capacity(1000);

        let alloc_block = |size| {
            // SAFETY: The block was allocated by an `Arena`.
            unsafe { Arena::make_transparent(arena.block_with_size(size).build()) }
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
        let arena = Arena::with_capacity(10_000);

        let alloc_block = |size| {
            // SAFETY: The block was allocated by an `Arena`.
            unsafe { Arena::make_transparent(arena.block_with_size(size).build()) }
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
