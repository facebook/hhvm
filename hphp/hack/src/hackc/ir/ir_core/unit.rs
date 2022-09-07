// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
use hash::HashMap;
pub use hhbc::ClassName;
pub use hhbc::ConstName;
pub use hhbc::FunctionName;
pub use hhbc::IncludePath;
pub use hhbc::Typedef;

use crate::constant::AdataId;
use crate::string_intern::StringInterner;
use crate::Attribute;
use crate::Class;
use crate::Function;
use crate::HackConstant;
use crate::Module;
use crate::SrcLoc;
use crate::TypedValue;

pub enum FatalOp<'a> {
    None,
    Parse(SrcLoc, Str<'a>),
    Runtime(SrcLoc, Str<'a>),
    RuntimeOmitFrame(SrcLoc, Str<'a>),
}

impl<'a> std::default::Default for FatalOp<'a> {
    fn default() -> Self {
        Self::None
    }
}

#[derive(Default)]
pub struct SymbolRefs<'a> {
    pub classes: Vec<ClassName<'a>>,
    pub constants: Vec<ConstName<'a>>,
    pub functions: Vec<FunctionName<'a>>,
    pub includes: Vec<IncludePath<'a>>,
}

/// Unit represents a single parsed file.
#[derive(Default)]
pub struct Unit<'a> {
    /// Mapping of internal name to array constants. Note that these are
    /// internal names, not user-defined.
    ///
    /// ```
    /// $a = vec[1, 2, 3];
    /// ```
    pub adata: HashMap<AdataId<'a>, TypedValue<'a>>,

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
    pub constants: Vec<HackConstant<'a>>,

    /// Per-file attributes.
    ///
    /// ```
    /// <<file:__EnableUnstableFeatures('readonly')>>
    /// ```
    pub file_attributes: Vec<Attribute<'a>>,

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
    pub fatal: FatalOp<'a>,

    pub modules: Vec<Module<'a>>,
    pub module_use: Option<Str<'a>>,

    /// The unit string interning table.
    pub strings: StringInterner,

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
    pub typedefs: Vec<Typedef<'a>>,
}
