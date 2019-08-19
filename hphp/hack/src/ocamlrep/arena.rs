// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::rc::Rc;

#[derive(Clone, PartialEq)]
struct Chunk {
    data: Box<[usize]>,
    index: RefCell<usize>,

    /// Pointer to the prev arena segment.
    prev: Option<Rc<Chunk>>,
}

impl Chunk {
    fn new_with_size(size: usize) -> Self {
        Self {
            index: RefCell::new(0),
            data: vec![0; size].into_boxed_slice(),
            prev: None,
        }
    }

    fn new_with_size_and_prev(size: usize, prev: Rc<Chunk>) -> Self {
        Self {
            index: RefCell::new(0),
            data: vec![0; size].into_boxed_slice(),
            prev: Some(prev),
        }
    }

    #[inline]
    pub fn alloc(&self, requested_size: usize) -> &[usize] {
        let previous_index = (*self.index.borrow_mut()).clone();
        *self.index.borrow_mut() += requested_size;
        &self.data[previous_index..(previous_index + requested_size)]
    }
}

pub struct Arena {
    current_chunk: Rc<Chunk>,
}

impl Arena {
    /// Allocates a new Arena with `initial_size` bytes preallocated.
    pub fn new_with_size(initial_size: usize) -> Self {
        Self {
            current_chunk: Rc::new(Chunk::new_with_size(initial_size)),
        }
    }

    #[inline]
    pub fn alloc(&mut self, requested_size: usize) -> &[usize] {
        if (*self.current_chunk.index.borrow()) + requested_size >= self.current_chunk.data.len() {
            self.current_chunk = Rc::new(Chunk::new_with_size_and_prev(
                std::cmp::max(requested_size * 2, self.current_chunk.data.len()),
                Rc::clone(&self.current_chunk),
            ));
        }
        self.current_chunk.alloc(requested_size)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    use std::time::Instant;

    #[test]
    fn test_alloc_ten() {
        let mut arena = Arena::new_with_size(1000);
        let get_ten = arena.alloc(10);

        assert_eq!(get_ten.len(), 10);
        assert_eq!(get_ten, &[0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
    }

    #[test]
    fn test_large_allocs() {
        let mut arena = Arena::new_with_size(1000);
        let get_max = arena.alloc(1000);
        assert_eq!(get_max.len(), 1000);

        let get_two_thousand = arena.alloc(2000);
        assert_eq!(get_two_thousand.len(), 2000);

        let get_four_thousand = arena.alloc(4000);
        assert_eq!(get_four_thousand.len(), 4000);
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
            arena.alloc(1);
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
            arena.alloc(5);
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
            arena.alloc(10);
        }
        println!("Arena: {:?}", now.elapsed());
    }
}
