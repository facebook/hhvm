// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use static_assertions::const_assert_eq;

// Compressed file (.zhhdg) file layout:
//
// ```txt
//  CompressedHeader
//  deps: [u64; num_deps]
//  edge_map: [varint; num_deps]
//    One varint-encoded integer per num_deps, indicating which edge array is used.
//    - Edge arrays appear in the adjacency_list section in order of first mention in the edge map.
//    - A value of 0 in the edge_map means this node is the first user of
//      an edge array, so that edge list is the next one in the adjacency list
//      section, and the decompressor implicitly knows which one that is.
//    - Any other value N means "this is a repeat use of an edge array, look
//      back N nodes in the edge map to find the most recent use of the same array."
//  adjacency_list: [u8; ...]
//    Each node's array of neighboring nodes has:
//    - varint length
//    - array of RLE blocks, where each RLE block is:
//        - varint+delta coded offset from 1 past the end of the previous block, or from 0 for first block,
//          except this number is doubled and the low bit set to indicate whether a repeat count follows.
//        - if the low bit above is set, then a varint encoded length minus two. Else length is implicitly one.
// ```
//
// Uncompressed file (.hhdg) file layout:
//
// ```txt
//  UncompressedHeader
//  deps: [u64; num_deps]
//  deps_sort_order: [u32; num_deps]
//  edge_map: [u32; num_deps]
//  adjacency_list: [struct { length: varint, rle_blocks: [RleBlock; length] }; ...]
// ```

/// Header in a .zhhdg file.
///
/// This is repr(C) so we can use its raw representation directly
/// from a memory-mapped file. Use explicit padding to avoid
/// "struct holes".
#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
pub struct CompressedHeader {
    pub magic: [u8; 4], // HHDZ
    pub version: u32,
    pub num_deps: u64,

    // Number of bytes used for the edge_map.
    pub edge_map_size: u64,
}

impl CompressedHeader {
    /// Magic number at the start of the file.
    pub const MAGIC: [u8; 4] = *b"HHDZ";

    /// Latest version number, in the file header.
    pub const LATEST_VERSION: u32 = 1;
}

const_assert_eq!(std::mem::size_of::<CompressedHeader>() % 8, 0);

/// Header in a .hhdg file.
///
/// This is repr(C) so we can use its raw representation directly
/// from a memory-mapped file. Use explicit padding to avoid
/// "struct holes".
#[repr(C)]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
pub struct UncompressedHeader {
    pub magic: [u8; 4],
    pub version: u32,
    pub num_deps: u64,

    /// Each edge list is stored at an offset in its section such that this many low bits
    /// of the offset are zero. This allows section offsets to be stored in 32 bits even if
    /// there are more than 4GB of edge lists.
    pub adjacency_list_alignment_shift: u8,

    pub _alignment_padding: [u8; 7],
}

impl UncompressedHeader {
    /// Magic number at the start of the file.
    pub const MAGIC: [u8; 4] = *b"HHDG";

    /// Latest version number, in the file header.
    pub const LATEST_VERSION: u32 = 1;
}

const_assert_eq!(std::mem::size_of::<UncompressedHeader>() % 8, 0);

/// This is a compact approximation for a u32 stored in a u8.
///
/// Any input <= 0x7f is represented precisely, but larger values are
/// approximated, (poorly, for values >= 2**27, but this is optimized for
/// smaller values). The approximation is never numerically larger than the
/// input.
///
/// This byte holds either a 7-bit value, or a 5-bit value that's left
/// shifted. Specifically, it's capable of representing any of the following
/// u32 values:
///
/// ```
/// 0000000000000000000000000xxxxxxx
/// 00000000000000000000xxxxx0000000
/// 000000000000000xxxxx000000000000
/// 0000000000xxxxx00000000000000000
/// 00000xxxxx0000000000000000000000
/// ```
///
/// Large values will often need to be represented as the sum of several encoded bytes.
///
/// The purpose of this function is to encode RLE block lengths using only a
/// single byte for the size (for compactness, because 0x7f is almost always
/// enough). We can't use a variable-length integer encoding because we need
/// a fixed size, so we can store RLE blocks in an array and binary search
/// them. When an RLE block size is so large we can't represent it exactly
/// as a `ByteApproximatedLen`, we create multiple consecutive RLE blocks
/// whose decoded sizes add up to the desired value; thus, the set of those
/// RLE blocks exactly covers the desired range.
///
/// Needing multiple blocks is not common so the overhead in practice is small.
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
#[repr(transparent)]
pub struct ByteApproximatedLen(u8);

impl ByteApproximatedLen {
    /// Return the largest encodable integer <= n.
    pub fn encode(n: u32) -> Self {
        let v = if n <= 0x7f {
            n as u8
        } else {
            let max_representable: u32 = 0x1f << Self::exp_to_shift(0x3);
            let n = std::cmp::min(n, max_representable);
            let exp = (4 - n.leading_zeros() / 5) as u8;
            let mantissa = (n >> Self::exp_to_shift(exp)) & 0x1f;
            0x80 | (exp << 5) | mantissa as u8
        };
        Self(v)
    }

    /// Convert this value back to the `u32` it represents.
    #[inline]
    pub fn decode(self) -> u32 {
        let n = self.0;
        if n <= 0x7f {
            n as u32
        } else {
            let exp = (n >> 5) & 0x3;
            let mantissa = n & 0x1f;
            (mantissa as u32) << Self::exp_to_shift(exp)
        }
    }

    /// Converts our two-bit exponent to a shift count.
    const fn exp_to_shift(exp: u8) -> u8 {
        // Shift past the low 7 bits, reserved for the <= 0x7f case.
        7 + 5 * exp
    }
}

/// A run-length-encoded consecutive range of integers.
///
/// This is packed so we can store them in memory-mapped files as arrays of 5-byte values.
#[repr(C, packed(1))]
#[derive(Clone, Copy, bytemuck::Pod, bytemuck::Zeroable)]
pub struct RleBlock {
    pub start: u32,
    pub encoded_len: ByteApproximatedLen,
}

impl RleBlock {
    #[inline]
    pub fn len(&self) -> u32 {
        self.encoded_len.decode()
    }

    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_byte_approximated_len() {
        // Encode all the values we can losslessly.
        for i in 0..128 {
            assert_eq!(i, ByteApproximatedLen::encode(i).decode());
        }
        for shift in 0..4 {
            for bits in 0..32 {
                let n = bits << (7 + shift * 5);
                assert_eq!(n, ByteApproximatedLen::encode(n).decode());
            }
        }

        // Clamp at the max.
        assert_eq!(ByteApproximatedLen::encode(!0u32).decode(), 0x1f << 22);

        // Decoded value should never be bigger than the input.
        for shift in 0..32 {
            let n = !0u32 >> shift;
            assert!(ByteApproximatedLen::encode(n).decode() <= n);
        }
    }
}
