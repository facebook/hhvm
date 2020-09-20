// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::Cell;
use std::result::Result;

#[derive(Debug)]
pub struct LineBreakMap {
    lines: Box<[usize]>,
    last_offset: Cell<usize>,
    curr_idnex: Cell<usize>,
}

impl LineBreakMap {
    pub fn new(text: &[u8]) -> Self {
        /* Clever Tricks Warning
         * ---------------------
         * We prepend 0, so as to make the invariant hold that there is always a
         * perceived line break at the start of the file. We use this in translating
         * offsets to line numbers.
         *
         * Similarly, whether there's a line break at the end or not, the line break
         * map will always end with the length of the original string. This solves
         * off-by-one issues.
         */
        let len = text.len();
        // Vec capacity grows exponentically, but it starts from 1.
        // Start with (len / 80) + 1, assuming 80 is average char count in a line.
        let mut result = Vec::with_capacity((len / 80) + 1);
        result.push(0);
        for i in 1..(len + 1) {
            let prev = text[i - 1];
            if prev == ('\r' as u8) && text[i] != ('\n' as u8) || prev == ('\n' as u8) {
                result.push(i);
            }
        }
        if result.len() > 1 {
            if let Some(i) = result.last() {
                if *i != len {
                    result.push(len);
                }
            }
        }
        LineBreakMap {
            lines: result.into_boxed_slice(),
            last_offset: Cell::new(0),
            curr_idnex: Cell::new(0),
        }
    }

    #[inline]
    fn to_isize(u: usize) -> isize {
        if (u as isize) < 0 {
            panic!("{} can not convert to isize", u.to_string())
        }
        u as isize
    }

    pub fn offset_to_file_pos_triple_original(&self, offset: isize) -> (isize, isize, isize) {
        if offset < 0 {
            return (1, 0, offset);
        }
        let (i, s, o) = self.offset_to_file_pos_triple(offset as usize);
        (Self::to_isize(i), Self::to_isize(s), Self::to_isize(o))
    }

    pub fn offset_to_file_pos_triple(&self, offset: usize) -> (usize, usize, usize) {
        let curr_index = self.curr_idnex.get();
        let last_offset = self.last_offset.get();
        let mut index = 0;
        if last_offset == offset {
            index = curr_index;
        } else {
            if offset > self.lines[curr_index] {
                index = self.lines.len() - 1;
                for i in curr_index..self.lines.len() {
                    if offset < self.lines[i] {
                        index = i - 1;
                        break;
                    }
                }
            } else {
                let mut i = curr_index;
                while i > 0 {
                    if self.lines[i] <= offset {
                        index = i;
                        break;
                    }
                    i -= 1;
                }
            }
            self.curr_idnex.set(index);
            self.last_offset.set(offset);
        }
        (index + 1, self.lines[index], offset)
    }

    pub fn offset_to_position(&self, offset: isize) -> (isize, isize) {
        let (index, line_start, offset) = self.offset_to_file_pos_triple_original(offset);
        (index, offset - line_start + 1)
    }

    pub fn position_to_offset(
        &self,
        existing: bool,
        line: isize,
        column: isize,
    ) -> Result<isize, ()> {
        fn get(v: &[usize], i: isize) -> isize {
            v[i as usize] as isize
        }

        let len = self.lines.len() as isize;
        let file_line = line;

        /* Treat all file_line errors the same */
        if file_line - 1 < 0 || file_line - 1 >= len {
            Err(())
        } else {
            let line_start = get(self.lines.as_ref(), file_line - 1);
            let offset = line_start + column - 1;

            if !existing
                || offset >= 0
                    && offset <= get(self.lines.as_ref(), std::cmp::min(len - 1, file_line))
            {
                Ok(offset)
            } else {
                Err(())
            }
        }
    }

    pub fn offset_to_line_start_offset(&self, offset: isize) -> isize {
        offset - self.offset_to_position(offset).1 + 1
    }

    pub fn offset_to_line_lengths(&self) -> Vec<isize> {
        panic!("Not implemented")
    }
}
