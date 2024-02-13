// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use bstr::BString;
use ffi::Str;

use crate::func::SrcLoc;
use crate::string_intern::StringInterner;
use crate::Attribute;
use crate::Class;
use crate::FatalOp;
use crate::Function;
use crate::HackConstant;
use crate::Module;
use crate::SymbolRefs;
use crate::Typedef;

/// Fields used when a unit had compile-time errors that should be reported
/// when the unit is loaded.
#[derive(Debug)]
pub struct Fatal {
    pub op: FatalOp,
    pub loc: SrcLoc,
    pub message: BString,
}

/// Unit represents a single parsed file.
#[derive(Debug, Default)]
pub struct Unit<'a> {
    /// The list of classes defined in this Unit. This also includes enums which
    /// are transformed into classes internally.
    ///
    /// ```
    /// class MyClass { ... }
    /// ```
    pub classes: Vec<Class<'a>>,

    /// The list of top-level constants.
    ///
    /// ```
    /// const MAGIC_VALUE: int = 42;
    /// ```
    pub constants: Vec<HackConstant>,

    /// Per-file attributes.
    ///
    /// ```
    /// <<file:__EnableUnstableFeatures('readonly')>>
    /// ```
    pub file_attributes: Vec<Attribute>,

    /// The list of top-level functions defined in this Unit. This include both
    /// user-defined functions and "special" compiler-defined functions (like
    /// static initializers).
    ///
    /// ```
    /// function my_fn(int $a, int $b) { ... }
    /// ```
    pub functions: Vec<Function<'a>>,

    /// If the Unit failed to parse or compile this defines the error that
    /// should be reported and the rest of the Unit will be empty.
    //
    // NB: It could be argued that Unit should be an enum with Fatal/Success
    // being two separate variants.
    pub fatal: Option<Fatal>,

    pub modules: Vec<Module>,
    pub module_use: Option<Str<'a>>,

    /// The unit string interning table.
    pub strings: Arc<StringInterner>,

    /// The list of all external symbols referenced by this Unit.
    //
    // NB: We really should be able to generate this from the IR itself instead
    // of relying on a separate table.
    pub symbol_refs: SymbolRefs<'a>,

    /// The list of top-level typedefs or aliases defined in this Unit.
    ///
    /// ```
    /// type A = B;
    /// ```
    pub typedefs: Vec<Typedef>,
}
