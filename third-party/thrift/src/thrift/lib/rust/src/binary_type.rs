/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use std::io::Cursor;

use bytes::Bytes;

use crate::bufext::BufExt;

/// Trait implemented on types that can be used as `binary` types in
/// thrift.  These types copy data from the Thrift buffer.
pub trait BinaryType: Sized {
    fn with_capacity(capacity: usize) -> Self;
    fn extend_from_slice(&mut self, other: &[u8]);
    fn from_vec(vec: Vec<u8>) -> Self {
        let mut binary = Self::with_capacity(vec.len());
        binary.extend_from_slice(&vec);
        binary
    }
}

/// Trait for copying from the Thrift buffer.  Special implementations
/// may do this without actually copying.
pub trait CopyFromBuf: Sized {
    fn copy_from_buf<B: BufExt>(buffer: &mut B, len: usize) -> Self;
    fn from_vec(vec: Vec<u8>) -> Self {
        Self::copy_from_buf(&mut Cursor::new(&vec), vec.len())
    }
}

impl BinaryType for Vec<u8> {
    fn with_capacity(capacity: usize) -> Self {
        Vec::with_capacity(capacity)
    }
    fn extend_from_slice(&mut self, other: &[u8]) {
        Vec::extend_from_slice(self, other);
    }
    fn from_vec(vec: Vec<u8>) -> Self {
        vec
    }
}

impl<T: BinaryType> CopyFromBuf for T {
    fn copy_from_buf<B: BufExt>(buffer: &mut B, len: usize) -> Self {
        assert!(buffer.remaining() >= len);
        let mut result = T::with_capacity(len);
        let mut remaining = len;

        while remaining > 0 {
            let part = buffer.chunk();
            let part_len = part.len().min(remaining);
            result.extend_from_slice(&part[..part_len]);
            remaining -= part_len;
            buffer.advance(part_len);
        }

        result
    }
    fn from_vec(vec: Vec<u8>) -> Self {
        BinaryType::from_vec(vec)
    }
}

pub(crate) struct Discard;

impl CopyFromBuf for Discard {
    fn copy_from_buf<B: BufExt>(buffer: &mut B, len: usize) -> Self {
        buffer.advance(len);
        Discard
    }
}

impl CopyFromBuf for Bytes {
    fn copy_from_buf<B: BufExt>(buffer: &mut B, len: usize) -> Self {
        buffer.copy_or_reuse_bytes(len)
    }
    fn from_vec(vec: Vec<u8>) -> Self {
        vec.into()
    }
}
