// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;

use crate::{BlockBuilder, OcamlRep, Value};

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
    pub fn alloc(&mut self, requested_size: usize) -> *mut Value<'a> {
        let previous_index = self.index;
        self.index += requested_size;
        &mut self.data[previous_index] as *mut Value<'a>
    }
}

pub struct Arena<'a> {
    current_chunk: RefCell<Chunk<'a>>,
}

impl<'a> Arena<'a> {
    /// Allocates a new Arena with `initial_size` bytes preallocated.
    pub fn new_with_size(initial_size: usize) -> Self {
        Self {
            current_chunk: RefCell::new(Chunk::new_with_size(initial_size)),
        }
    }

    #[inline]
    fn alloc(&self, requested_size: usize) -> &mut [Value<'a>] {
        if !self.current_chunk.borrow().can_fit(requested_size) {
            let prev_chunk_capacity = self.current_chunk.borrow().capacity();
            let prev_chunk = self
                .current_chunk
                .replace(Chunk::new_with_size(std::cmp::max(
                    requested_size * 2,
                    prev_chunk_capacity,
                )));
            self.current_chunk.borrow_mut().prev = Some(Box::new(prev_chunk));
        }
        let ptr = self.current_chunk.borrow_mut().alloc(requested_size);
        unsafe { std::slice::from_raw_parts_mut(ptr, requested_size) }
    }

    #[inline]
    pub fn block_with_size<'b>(&'b self, size: usize) -> BlockBuilder<'a, 'b> {
        self.block_with_size_and_tag(size, 0)
    }

    #[inline]
    pub fn block_with_size_and_tag<'b>(&'b self, size: usize, tag: u8) -> BlockBuilder<'a, 'b> {
        let slice = self.alloc(size + 1);
        BlockBuilder::new(size, tag, slice)
    }

    #[inline]
    pub fn add<T: OcamlRep>(&self, value: T) -> Value<'a> {
        value.into_ocamlrep(self)
    }

    pub unsafe fn add_from_ocaml(&self, value: usize) -> Value<'a> {
        use crate::block::{DOUBLE_TAG, STRING_TAG};
        if value & 1 == 1 {
            return Value::bits(value);
        }
        let ptr = value as *const Value<'a>;
        let ptr = ptr.offset(-1);
        let header = *ptr;
        let size = header.0 >> 10;
        let tag = header.0 as u8;
        let block = self.alloc(size + 1).as_mut_ptr();
        // Copy header
        *block = *ptr;
        if tag == STRING_TAG || tag == DOUBLE_TAG {
            // Copy binary data
            std::ptr::copy_nonoverlapping(ptr, block, size + 1);
        } else {
            // Copy fields
            for i in 1..=(size as isize) {
                *block.offset(i) = self.add_from_ocaml((*ptr.offset(i)).0);
            }
        }
        Value::bits(std::mem::transmute(block.offset(1)))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::time::Instant;

    #[test]
    fn test_alloc_block_of_three_fields() {
        let arena = Arena::new_with_size(1000);
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
        let arena = Arena::new_with_size(1000);
        let max = arena.block_with_size(1000).build().as_block().unwrap();
        assert_eq!(max.size(), 1000);

        let two_thousand = arena.block_with_size(2000).build().as_block().unwrap();
        assert_eq!(two_thousand.size(), 2000);

        let four_thousand = arena.block_with_size(4000).build().as_block().unwrap();
        assert_eq!(four_thousand.size(), 4000);
    }

    #[test]
    fn perf_test() {
        let arena = Arena::new_with_size(10_000);

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
