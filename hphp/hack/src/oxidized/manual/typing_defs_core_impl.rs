// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_defs_core::ConsistentKind;

impl ConsistentKind {
    /// If the parent's constructor is consistent via `<<__ConsistentConstruct>>`,
    /// then we want to carry this forward even if the child is final. Example:
    ///
    /// ```
    /// <<__ConsistentConstruct>>
    /// class C {
    ///   public static function f(): void {
    ///     new static();
    ///   }
    /// }
    /// final class D<reify T> extends C {}
    /// ```
    ///
    /// Even though D's consistency locally comes from the fact that D was
    /// declared as a final class, calling `new static()` via `D::f` will cause
    /// a runtime exception because D has reified generics.
    ///
    /// c.f. OCaml function `Decl_utils.coalesce_consistent`
    pub fn coalesce(parent: Self, current: Self) -> Self {
        match parent {
            Self::Inconsistent => current,
            Self::ConsistentConstruct => parent,
            // This case is unreachable in a correct program, because parent
            // would have to be a final class
            Self::FinalClass => parent,
        }
    }
}
