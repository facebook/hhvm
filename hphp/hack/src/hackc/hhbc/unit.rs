// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Vector;
use serde::Serialize;

use crate::Attribute;
use crate::ClassImpl;
use crate::Constant;
use crate::FatalOp;
use crate::FunctionImpl;
use crate::Module;
use crate::ModuleName;
use crate::SrcLoc;
use crate::StringId;
use crate::SymbolRefs;
use crate::Typedef;

pub type Unit = UnitImpl<crate::BcRepr>;

/// Unit represents a single compiled file.
#[derive(Debug, Default, Clone, Serialize)]
#[repr(C)]
pub struct UnitImpl<R> {
    /// The list of top-level functions defined in this Unit. This include both
    /// user-defined functions and "special" compiler-defined functions (like
    /// static initializers).
    ///
    /// ```
    /// function my_fn(int $a, int $b) { ... }
    /// ```
    pub functions: Vector<FunctionImpl<R>>,

    /// The list of classes defined in this Unit. This also includes enums which
    /// are transformed into classes internally.
    ///
    /// ```
    /// class MyClass { ... }
    /// ```
    pub classes: Vector<ClassImpl<R>>,

    /// What modules are defined in this unit
    pub modules: Vector<Module>,

    /// The list of top-level typedefs or aliases defined in this Unit.
    ///
    /// ```
    /// type A = B;
    /// ```
    pub typedefs: Vector<Typedef>,

    /// Per-file attributes.
    ///
    /// ```
    /// <<file:__EnableUnstableFeatures('readonly')>>
    /// ```
    pub file_attributes: Vector<Attribute>,

    /// What module is this unit declared to be part of
    pub module_use: Maybe<ModuleName>,

    /// The list of all external symbols referenced by this Unit.
    // NB: We really should be able to generate this from the IR itself instead
    // of relying on a separate table.
    pub symbol_refs: SymbolRefs,

    /// The list of top-level constants.
    ///
    /// ```
    /// const MAGIC_VALUE: int = 42;
    /// ```
    pub constants: Vector<Constant>,

    /// If the Unit failed to parse or compile this defines the error that
    /// should be reported and the rest of the Unit will be empty.
    // NB: It could be argued that Unit should be an enum with Fatal/Success
    // being two separate variants.
    pub fatal: Maybe<Fatal>,

    pub missing_symbols: Vector<StringId>,
    pub error_symbols: Vector<StringId>,
}

/// Fields used when a unit had compile-time errors that should be reported
/// when the unit is loaded.
#[derive(Debug, Clone, Serialize)]
#[repr(C)]
pub struct Fatal {
    pub op: FatalOp,
    pub loc: SrcLoc,
    pub message: Vector<u8>,
}
