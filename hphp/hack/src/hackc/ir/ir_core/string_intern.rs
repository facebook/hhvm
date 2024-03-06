// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use bstr::BStr;
use intern::string::BytesId;
pub type UnitBytesId = BytesId;
pub type UnitBytesIdMap<V> = intern::string::BytesIdMap<V>;
pub type UnitBytesIdSet = intern::string::BytesIdSet;
use intern::InternId;

/// A string interner for associating IDs with unique string values.  If two
/// identical strings are inserted into the StringInterner they are guaranteed
/// to have the same UnitBytesId.
///
/// Note that there are no guarantees about the numerical values or ordering of
/// the resulting UnitBytesId - in particular use of StringInterner in
/// multi-thread situations will produce non-deterministic ID ordering.
///
/// Currently there is no easy facility to iterate the strings in-order - this
/// prevents accidental ordering misuse.
#[derive(Default)]
pub struct StringInterner;

impl StringInterner {
    pub fn read_only() -> Self {
        Self
    }

    pub fn intern_bytes<'b>(&self, s: impl Into<Cow<'b, [u8]>>) -> UnitBytesId {
        intern::string::intern_bytes(s.into().as_ref())
    }

    // TODO: This should return UnitStringId
    pub fn intern_str<'b>(&self, s: impl Into<Cow<'b, str>>) -> UnitBytesId {
        intern::string::intern_bytes(s.into().as_ref().as_bytes())
    }

    pub fn is_empty(&self) -> bool {
        BytesId::table().is_empty()
    }

    pub fn len(&self) -> usize {
        BytesId::table().len()
    }

    pub fn lookup_bytes(&self, id: UnitBytesId) -> &[u8] {
        id.as_bytes()
    }

    pub fn lookup_bytes_or_none(&self, id: UnitBytesId) -> Option<&[u8]> {
        if id == UnitBytesId::EMPTY {
            None
        } else {
            Some(self.lookup_bytes(id))
        }
    }

    pub fn lookup_bstr(&self, id: UnitBytesId) -> &BStr {
        self.lookup_bytes(id).into()
    }

    pub fn eq_str(&self, id: UnitBytesId, rhs: &str) -> bool {
        self.lookup_bytes(id) == rhs.as_bytes()
    }

    pub fn display(&self, id: UnitBytesId) -> impl std::fmt::Display {
        struct Displayer(UnitBytesId);
        impl std::fmt::Display for Displayer {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                StringInterner.lookup_bstr(self.0).fmt(f)
            }
        }
        Displayer(id)
    }
}

impl std::fmt::Debug for StringInterner {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str("StringInterner")
    }
}
