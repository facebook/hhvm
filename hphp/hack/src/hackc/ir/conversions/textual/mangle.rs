// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::StringInterner;

pub(crate) trait Mangle {
    fn mangle(&self) -> String;
}

pub(crate) trait MangleId {
    fn mangle(&self, strings: &StringInterner) -> String;
}

impl Mangle for [u8] {
    fn mangle(&self) -> String {
        // [A-Za-z0-9_] -> identity
        // \ -> ::
        // anything else -> $xx
        let mut res = String::with_capacity(self.len());
        for &ch in self {
            if (b'A'..=b'Z').contains(&ch)
                || (b'a'..=b'z').contains(&ch)
                || (b'0'..=b'9').contains(&ch)
                || (ch == b'_')
            {
                res.push(ch as char);
            } else if ch == b'\\' {
                res.push(':');
                res.push(':');
            } else {
                res.push(b"0123456789abcdef"[(ch >> 4) as usize] as char);
                res.push(b"0123456789abcdef"[(ch & 15) as usize] as char);
            }
        }
        res
    }
}

impl Mangle for ir::FunctionName<'_> {
    fn mangle(&self) -> String {
        format!("_H{}", self.as_bytes().mangle())
    }
}

impl MangleId for ir::FunctionId {
    fn mangle(&self, strings: &StringInterner) -> String {
        format!("_H{}", self.as_bytes(strings).mangle())
    }
}
