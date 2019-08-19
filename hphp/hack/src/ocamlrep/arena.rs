// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{BlockBuilder, IntoOcamlRep, Value};

struct Chunk<'a> {
    data: Box<[Value<'a>]>,
    index: usize,

    /// Pointer to the prev arena segment.
    prev: Option<Box<Chunk<'a>>>,
}

impl<'a> Chunk<'a> {
    fn new_with_size(size: usize) -> Self {
        Self {
            index: 0,
            data: vec![Value::int(0); size].into_boxed_slice(),
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
    pub fn alloc(&mut self, requested_size: usize) -> &mut [Value<'a>] {
        let previous_index = self.index;
        self.index += requested_size;
        &mut self.data[previous_index..self.index]
    }
}

pub struct Arena<'a> {
    current_chunk: Chunk<'a>,
}

impl<'a> Arena<'a> {
    /// Allocates a new Arena with `initial_size` bytes preallocated.
    pub fn new_with_size(initial_size: usize) -> Self {
        Self {
            current_chunk: Chunk::new_with_size(initial_size),
        }
    }

    #[inline]
    fn alloc(&mut self, requested_size: usize) -> &mut [Value<'a>] {
        if !self.current_chunk.can_fit(requested_size) {
            let prev_chunk_capacity = self.current_chunk.capacity();
            let prev_chunk = std::mem::replace(
                &mut self.current_chunk,
                Chunk::new_with_size(std::cmp::max(requested_size * 2, prev_chunk_capacity)),
            );
            self.current_chunk.prev = Some(Box::new(prev_chunk));
        }
        self.current_chunk.alloc(requested_size)
    }

    #[inline]
    pub fn block_with_size<'b>(&'b mut self, size: usize) -> BlockBuilder<'a, 'b> {
        self.block_with_size_and_tag(size, 0)
    }

    #[inline]
    pub fn block_with_size_and_tag<'b>(&'b mut self, size: usize, tag: u8) -> BlockBuilder<'a, 'b> {
        let slice = self.alloc(size + 1);
        BlockBuilder::new(size, tag, slice)
    }

    #[inline]
    pub fn add<T: IntoOcamlRep>(&mut self, value: T) -> Value<'a> {
        value.into_ocamlrep(self)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::time::Instant;

    #[test]
    fn test_alloc_block_of_three_fields() {
        let mut arena = Arena::new_with_size(1000);
        let mut block = arena.block_with_size(3);
        block[0] = Value::int(1);
        block[1] = Value::int(2);
        block[2] = Value::int(3);
        let block = block.build().as_block().unwrap();

        assert_eq!(block.size(), 3);
        assert_eq!(block[0].as_int(), 1);
        assert_eq!(block[1].as_int(), 2);
        assert_eq!(block[2].as_int(), 3);
    }

    #[test]
    fn test_large_allocs() {
        let mut arena = Arena::new_with_size(1000);
        let max = arena.block_with_size(1000).build().as_block().unwrap();
        assert_eq!(max.size(), 1000);

        let two_thousand = arena.block_with_size(2000).build().as_block().unwrap();
        assert_eq!(two_thousand.size(), 2000);

        let four_thousand = arena.block_with_size(4000).build().as_block().unwrap();
        assert_eq!(four_thousand.size(), 4000);
    }

    #[test]
    fn perf_test() {
        let mut arena = Arena::new_with_size(10_000);

        println!("Benchmarks for allocating [1] 200,000 times");
        let now = Instant::now();
        for _ in 0..200_000 {
            vec![0; 1].into_boxed_slice();
        }
        println!("Alloc: {:?}", now.elapsed());

        let now = Instant::now();
        for _ in 0..200_000 {
            arena.block_with_size(1).build().as_block().unwrap();
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
            arena.block_with_size(5).build().as_block().unwrap();
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
            arena.block_with_size(10).build().as_block().unwrap();
        }
        println!("Arena: {:?}", now.elapsed());
    }
}
