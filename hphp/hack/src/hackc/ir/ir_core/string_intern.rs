// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use bstr::BStr;
use hash::IndexSet;
use newtype::newtype_int;

// Improvement list:
//
// - In debug mode have UnitBytesId store a pointer to the original
// StringInterner and check that whenever the UnitBytesId is used to look up
// values in the table. It's not as safe as using lifetimes to track it but it's
// a lot cleaner code.

// A UnitBytesId represents an entry in the Unit::strings table.
newtype_int!(UnitBytesId, u32, UnitBytesIdMap, UnitBytesIdSet);

#[derive(Default)]
pub struct StringInterner {
    values: IndexSet<Vec<u8>>,
}

impl StringInterner {
    pub fn intern_bytes<'b>(&mut self, s: impl Into<Cow<'b, [u8]>>) -> UnitBytesId {
        let s = s.into();
        let index = self
            .values
            .get_index_of(s.as_ref())
            .unwrap_or_else(|| self.values.insert_full(s.into_owned()).0);
        UnitBytesId::from_usize(index)
    }

    // TODO: This should return UnitStringId
    pub fn intern_str<'b>(&mut self, s: impl Into<Cow<'b, str>>) -> UnitBytesId {
        let s = s.into();
        match s {
            Cow::Owned(s) => self.intern_bytes(s.into_bytes()),
            Cow::Borrowed(s) => self.intern_bytes(s.as_bytes()),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.values.is_empty()
    }

    pub fn len(&self) -> usize {
        self.values.len()
    }

    pub fn lookup_bytes(&self, id: UnitBytesId) -> &[u8] {
        &self.values[id.as_usize()]
    }

    pub fn lookup_bstr(&self, id: UnitBytesId) -> &BStr {
        self.lookup_bytes(id).into()
    }
}
