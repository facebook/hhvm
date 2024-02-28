// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use bstr::BStr;
use hash::IndexSet;
use newtype::newtype_int;
pub use parking_lot::MappedRwLockReadGuard;
use parking_lot::RwLock;
use parking_lot::RwLockReadGuard;

// Improvement list:
//
// - In debug mode have UnitBytesId store a pointer to the original
// StringInterner and check that whenever the UnitBytesId is used to look up
// values in the table. It's not as safe as using lifetimes to track it but it's
// a lot cleaner code.

// A UnitBytesId represents an entry in the Unit::strings table.
newtype_int!(UnitBytesId, u32, UnitBytesIdMap, UnitBytesIdSet);

impl UnitBytesId {
    pub fn display<'a>(self, strings: &'a StringInterner) -> impl std::fmt::Display + 'a {
        struct D<'a>(UnitBytesId, &'a StringInterner);
        impl std::fmt::Display for D<'_> {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                self.1.lookup_bstr(self.0).fmt(f)
            }
        }
        D(self, strings)
    }

    pub fn is_empty(self, strings: &StringInterner) -> bool {
        strings.eq_str(self, "")
    }
}

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
pub struct StringInterner {
    values: RwLock<IndexSet<Vec<u8>>>,
    read_only: bool,
}

impl StringInterner {
    pub fn read_only() -> StringInterner {
        StringInterner {
            values: Default::default(),
            read_only: true,
        }
    }

    pub fn intern_bytes<'b>(&self, s: impl Into<Cow<'b, [u8]>>) -> UnitBytesId {
        let s = s.into();
        // We could use an upgradable_read() - but there's only one of those
        // allowed at a time so we'd lose read concurrency.
        let values = self.values.read();
        if let Some(index) = values.get_index_of(s.as_ref()) {
            return UnitBytesId::from_usize(index);
        }
        drop(values);
        assert!(!self.read_only);
        let mut values = self.values.write();
        UnitBytesId::from_usize(if let Some(index) = values.get_index_of(s.as_ref()) {
            index
        } else {
            values.insert_full(s.into_owned()).0
        })
    }

    // TODO: This should return UnitStringId
    pub fn intern_str<'b>(&self, s: impl Into<Cow<'b, str>>) -> UnitBytesId {
        let s = s.into();
        match s {
            Cow::Owned(s) => self.intern_bytes(s.into_bytes()),
            Cow::Borrowed(s) => self.intern_bytes(s.as_bytes()),
        }
    }

    pub fn is_empty(&self) -> bool {
        self.values.read().is_empty()
    }

    pub fn len(&self) -> usize {
        self.values.read().len()
    }

    pub fn lookup_bytes<'a>(&'a self, id: UnitBytesId) -> MappedRwLockReadGuard<'a, [u8]> {
        assert!(id != UnitBytesId::NONE);
        let values = self.values.read();
        RwLockReadGuard::map(values, |values| -> &[u8] { &values[id.as_usize()] })
    }

    pub fn lookup_bytes_or_none<'a>(
        &'a self,
        id: UnitBytesId,
    ) -> Option<MappedRwLockReadGuard<'a, [u8]>> {
        if id == UnitBytesId::NONE {
            None
        } else {
            Some(self.lookup_bytes(id))
        }
    }

    pub fn lookup_bstr<'a>(&'a self, id: UnitBytesId) -> MappedRwLockReadGuard<'a, BStr> {
        MappedRwLockReadGuard::map(self.lookup_bytes(id), |v: &[u8]| -> &BStr { v.into() })
    }

    pub fn eq_str(&self, id: UnitBytesId, rhs: &str) -> bool {
        let lhs = self.lookup_bytes(id);
        lhs.as_ref() as &[u8] == rhs.as_bytes()
    }

    /// This is meant for debugging ONLY.
    pub fn debug_for_each<F>(&self, mut f: F)
    where
        F: FnMut(UnitBytesId, &[u8]),
    {
        self.values
            .read()
            .iter()
            .enumerate()
            .for_each(|(idx, s)| f(UnitBytesId::from_usize(idx), s))
    }
}

impl std::fmt::Debug for StringInterner {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str("{intern}")
    }
}
