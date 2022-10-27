// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ir::StringInterner;

const TOP_LEVELS_CLASS: &str = "$root";

/// Used for things that can mangle themselves directly.
pub(crate) trait Mangle {
    fn mangle(&self, strings: &StringInterner) -> String;
}

pub(crate) trait ManglableClass {
    fn mangle_class(&self, strings: &StringInterner) -> String;
}

impl ManglableClass for ir::ClassId {
    fn mangle_class(&self, strings: &StringInterner) -> String {
        self.mangle(strings)
    }
}

impl ManglableClass for &ir::unit::ClassName<'_> {
    fn mangle_class(&self, strings: &StringInterner) -> String {
        self.mangle(strings)
    }
}

/// Used for things that need to be mangled relative to a Class.
pub(crate) trait MangleWithClass {
    fn mangle(&self, class: impl ManglableClass, strings: &StringInterner) -> String;
}

impl Mangle for [u8] {
    fn mangle(&self, _strings: &StringInterner) -> String {
        // [A-Za-z0-9_$] -> identity
        // \ -> ::
        // anything else -> $xx
        let mut res = String::with_capacity(self.len());
        for &ch in self {
            if (b'A'..=b'Z').contains(&ch)
                || (b'a'..=b'z').contains(&ch)
                || (b'0'..=b'9').contains(&ch)
                || (ch == b'_')
                || (ch == b'$')
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

impl Mangle for ir::FunctionName<'_> {
    fn mangle(&self, strings: &StringInterner) -> String {
        format!("{TOP_LEVELS_CLASS}.{}", self.as_bytes().mangle(strings))
    }
}

impl Mangle for ir::ClassId {
    fn mangle(&self, strings: &StringInterner) -> String {
        self.as_bytes(strings).mangle(strings)
    }
}

impl Mangle for ir::unit::ClassName<'_> {
    fn mangle(&self, strings: &StringInterner) -> String {
        self.as_bytes().mangle(strings)
    }
}

impl Mangle for ir::FunctionId {
    fn mangle(&self, strings: &StringInterner) -> String {
        format!(
            "{TOP_LEVELS_CLASS}.{}",
            self.as_bytes(strings).mangle(strings)
        )
    }
}

impl MangleWithClass for ir::MethodName<'_> {
    fn mangle(&self, class: impl ManglableClass, strings: &StringInterner) -> String {
        format!(
            "{}.{}",
            class.mangle_class(strings),
            self.as_bytes().mangle(strings)
        )
    }
}

impl MangleWithClass for ir::MethodId {
    fn mangle(&self, class: impl ManglableClass, strings: &StringInterner) -> String {
        format!(
            "{}.{}",
            class.mangle_class(strings),
            self.as_bytes(strings).mangle(strings)
        )
    }
}
