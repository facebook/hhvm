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

impl ManglableClass for &ir::ClassName<'_> {
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
        // Handle some reserved tokens.
        match self {
            b"declare" => "declare_".to_owned(),
            b"define" => "define_".to_owned(),
            b"extends" => "extends_".to_owned(),
            b"false" => "false_".to_owned(),
            b"float" => "float_".to_owned(),
            b"global" => "global_".to_owned(),
            b"int" => "int_".to_owned(),
            b"jmp" => "jmp_".to_owned(),
            b"load" => "load_".to_owned(),
            b"local" => "local_".to_owned(),
            b"null" => "null_".to_owned(),
            b"prune" => "prune_".to_owned(),
            b"ret" => "ret_".to_owned(),
            b"store" => "store_".to_owned(),
            b"throw" => "throw_".to_owned(),
            b"true" => "true_".to_owned(),
            b"type" => "type_".to_owned(),
            b"unreachable" => "unreachable_".to_owned(),
            b"void" => "void_".to_owned(),
            _ => {
                // [A-Za-z0-9_$] -> identity
                // \ -> ::
                // anything else -> $xx
                let mut res = String::with_capacity(self.len());
                let mut first = true;
                for &ch in self {
                    if (b'A'..=b'Z').contains(&ch)
                        || (b'a'..=b'z').contains(&ch)
                        || (ch == b'_')
                        || (ch == b'$')
                    {
                        res.push(ch as char);
                    } else if (b'0'..=b'9').contains(&ch) {
                        if first {
                            res.push('_')
                        }
                        res.push(ch as char);
                    } else if ch == b'\\' {
                        res.push(':');
                        res.push(':');
                    } else {
                        res.push(b"0123456789abcdef"[(ch >> 4) as usize] as char);
                        res.push(b"0123456789abcdef"[(ch & 15) as usize] as char);
                    }
                    first = false;
                }
                res
            }
        }
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

impl Mangle for ir::ClassName<'_> {
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

impl Mangle for ir::PropId {
    fn mangle(&self, strings: &StringInterner) -> String {
        self.as_bytes(strings).mangle(strings)
    }
}
