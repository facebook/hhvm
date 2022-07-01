// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bstr::BStr;
use ffi::Str;
use hash::HashMap;
use newtype::newtype_int;
use newtype::IdVec;

// Improvement list:
//
// - In debug mode have UnitStringId store a pointer to the original
// StringInterner and check that whenever the UnitStringId is used to look up
// values in the table. It's not as safe as using lifetimes to track it but it's
// a lot cleaner code.

pub enum InternValue<'a> {
    Str(Str<'a>),
}

impl<'a> InternValue<'a> {
    pub fn as_bstr(&self) -> &BStr {
        match self {
            InternValue::Str(s) => s.as_bstr(),
        }
    }

    pub fn as_bytes(&self) -> &[u8] {
        match self {
            InternValue::Str(s) => s.as_ref(),
        }
    }

    pub fn to_ffi_str(&self, _alloc: &'a bumpalo::Bump) -> Str<'a> {
        match self {
            InternValue::Str(s) => *s,
        }
    }
}

// A UnitStringId represents an entry in the Unit::strings table.
newtype_int!(UnitStringId, u32, UnitStringIdMap, UnitStringIdSet);

#[derive(Default)]
pub struct StringInterner<'a> {
    lookup: HashMap<Str<'a>, UnitStringId>,
    table: IdVec<UnitStringId, InternValue<'a>>,
}

impl<'a> StringInterner<'a> {
    pub fn intern_str(&mut self, s: Str<'a>) -> UnitStringId {
        *self.lookup.entry(s).or_insert_with(|| {
            let idx = self.table.len();
            let v = InternValue::Str(s);
            self.table.push(v);
            UnitStringId::from_usize(idx)
        })
    }

    pub fn lookup(&self, id: UnitStringId) -> &InternValue<'a> {
        &self.table[id]
    }
}
