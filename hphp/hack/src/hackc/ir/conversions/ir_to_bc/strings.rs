// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::StringInterner;
use ir::UnitBytesId;

pub(crate) struct StringCache {
    pub interner: Arc<StringInterner>,
}

impl StringCache {
    pub fn new(interner: Arc<StringInterner>) -> Self {
        Self { interner }
    }

    pub fn intern(&self, id: UnitBytesId) -> Result<hhbc::StringId, std::str::Utf8Error> {
        Ok(hhbc::intern(std::str::from_utf8(
            &self.interner.lookup_bytes(id),
        )?))
    }

    pub fn lookup_class_name(&self, id: ir::ClassId) -> hhbc::ClassName {
        hhbc::ClassName::intern(
            std::str::from_utf8(&self.interner.lookup_bstr(id.id)).expect("non-utf8 class name"),
        )
    }

    pub fn lookup_module_name(&self, id: ir::ModuleId) -> hhbc::ModuleName {
        hhbc::ModuleName::intern(
            std::str::from_utf8(&self.interner.lookup_bstr(id.id)).expect("non-utf8 module name"),
        )
    }

    pub fn lookup_const_name(&self, id: ir::ConstId) -> hhbc::ConstName {
        hhbc::ConstName::intern(
            std::str::from_utf8(&self.interner.lookup_bstr(id.id)).expect("non-utf8 const name"),
        )
    }

    pub fn lookup_method_name(&self, id: ir::MethodId) -> hhbc::MethodName {
        hhbc::MethodName::intern(
            std::str::from_utf8(&self.interner.lookup_bstr(id.id)).expect("non-utf8 method name"),
        )
    }

    pub fn lookup_function_name(&self, id: ir::FunctionId) -> hhbc::FunctionName {
        hhbc::FunctionName::intern(
            std::str::from_utf8(&self.interner.lookup_bstr(id.id)).expect("non-utf8 function name"),
        )
    }

    pub fn lookup_prop_name(&self, id: ir::PropId) -> hhbc::PropName {
        hhbc::PropName::intern(
            std::str::from_utf8(&self.interner.lookup_bstr(id.id)).expect("non-utf8 prop name"),
        )
    }
}
