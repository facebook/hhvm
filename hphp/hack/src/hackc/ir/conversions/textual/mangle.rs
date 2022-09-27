// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::ClassId;
use ir::StringInterner;

/// Used for things that can mangle themselves directly.
pub(crate) trait Mangle {
    fn mangle(&self) -> String;
}

/// Used for things that need a StringInterner to mangle themselves.
pub(crate) trait MangleId {
    fn mangle(&self, strings: &StringInterner) -> String;
}

/// Used for things that need to be mangled relative to a ClassId.
pub(crate) trait MangleClassId {
    fn mangle(&self, class: ClassId, strings: &StringInterner) -> String;
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

// Classes and functions live in different namespaces.

fn mangle_func_name(func: &[u8]) -> String {
    format!("_H{}", func.mangle())
}

fn mangle_class_name(class: &[u8]) -> String {
    format!("_C{}", class.mangle())
}

fn mangle_method_name(class: &[u8], method: &[u8]) -> String {
    format!("_M{}::{}", class.mangle(), method.mangle())
}

impl Mangle for ir::FunctionName<'_> {
    fn mangle(&self) -> String {
        mangle_func_name(self.as_bytes())
    }
}

impl MangleId for ir::ClassId {
    fn mangle(&self, strings: &StringInterner) -> String {
        mangle_class_name(self.as_bytes(strings))
    }
}

impl MangleId for ir::FunctionId {
    fn mangle(&self, strings: &StringInterner) -> String {
        mangle_func_name(self.as_bytes(strings))
    }
}

impl MangleClassId for ir::MethodName<'_> {
    fn mangle(&self, class: ClassId, strings: &StringInterner) -> String {
        mangle_method_name(class.as_bytes(strings), self.as_bytes())
    }
}

impl MangleClassId for ir::MethodId {
    fn mangle(&self, class: ClassId, strings: &StringInterner) -> String {
        mangle_method_name(class.as_bytes(strings), self.as_bytes(strings))
    }
}
