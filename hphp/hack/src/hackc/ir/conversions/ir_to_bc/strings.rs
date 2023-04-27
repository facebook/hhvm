// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use dashmap::DashMap;
use ffi::Str;
use ir::StringInterner;
use ir::UnitBytesId;

pub(crate) struct StringCache<'a> {
    pub alloc: &'a bumpalo::Bump,
    cache: DashMap<UnitBytesId, Str<'a>>,
    pub interner: Arc<StringInterner>,
}

impl<'a> StringCache<'a> {
    pub fn new(alloc: &'a bumpalo::Bump, interner: Arc<StringInterner>) -> Self {
        let cache = DashMap::with_capacity(interner.len());
        Self {
            alloc,
            cache,
            interner,
        }
    }

    pub fn lookup_ffi_str(&self, id: UnitBytesId) -> Str<'a> {
        *self.cache.entry(id).or_insert_with(|| {
            let s = self.interner.lookup_bstr(id);
            Str::new_slice(self.alloc, &s)
        })
    }

    pub fn lookup_class_name(&self, id: ir::ClassId) -> hhbc::ClassName<'a> {
        let s = self.lookup_ffi_str(id.id);
        hhbc::ClassName::new(s)
    }

    pub fn lookup_const_name(&self, id: ir::ConstId) -> hhbc::ConstName<'a> {
        let s = self.lookup_ffi_str(id.id);
        hhbc::ConstName::new(s)
    }

    pub fn lookup_method_name(&self, id: ir::MethodId) -> hhbc::MethodName<'a> {
        let s = self.lookup_ffi_str(id.id);
        hhbc::MethodName::new(s)
    }

    pub fn lookup_function_name(&self, id: ir::FunctionId) -> hhbc::FunctionName<'a> {
        let s = self.lookup_ffi_str(id.id);
        hhbc::FunctionName::new(s)
    }

    pub fn lookup_prop_name(&self, id: ir::PropId) -> hhbc::PropName<'a> {
        let s = self.lookup_ffi_str(id.id);
        hhbc::PropName::new(s)
    }
}
