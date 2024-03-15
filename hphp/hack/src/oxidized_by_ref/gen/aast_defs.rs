// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<de77615a174ee02984a87f773d4ab905>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
pub use ast_defs::Pos;
pub use ast_defs::PositionedByteString;
pub use ast_defs::Pstring;
pub use local_id::LocalId;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
pub use oxidized::aast_defs::OgNullFlavor;
pub use oxidized::aast_defs::PropOrMethod;
pub use oxidized::aast_defs::ReifyKind;
pub use oxidized::aast_defs::Tprim;
pub use oxidized::aast_defs::TypedefVisibility;
pub use oxidized::aast_defs::Visibility;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C)]
pub struct Lid<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a LocalId<'a>,
);
impl<'a> TrivialDrop for Lid<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Lid<'arena>);

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
pub type Sid<'a> = ast_defs::Id<'a>;

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
pub type ClassName<'a> = Sid<'a>;

/// Aast.program represents the top-level definitions in a Hack program.
/// ex: Expression annotation type (when typechecking, the inferred type)
/// en: Environment (tracking state inside functions and classes)
#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Program<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a [Def<'a, Ex, En>],
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Program<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Program<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Stmt<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Stmt_<'a, Ex, En>,
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Stmt<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Stmt<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Stmt_<'a, Ex, En> {
    /// No-op, the empty statement.
    ///
    ///     {}
    ///     while (true) ;
    ///     if ($foo) {} // the else is Noop here
    Noop,
    /// Marker for a switch statement that falls through.
    ///
    ///     // FALLTHROUGH
    Fallthrough,
    /// Standalone expression.
    ///
    ///     1 + 2;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Expr(&'a Expr<'a, Ex, En>),
    /// Break inside a loop or switch statement.
    ///
    ///     break;
    Break,
    /// Continue inside a loop or switch statement.
    ///
    ///     continue;
    Continue,
    /// Throw an exception.
    ///
    /// throw $foo;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Throw(&'a Expr<'a, Ex, En>),
    /// Return, with an optional value.
    ///
    ///     return;
    ///     return $foo;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Return(Option<&'a Expr<'a, Ex, En>>),
    /// Yield break, terminating the current generator. This behaves like
    /// return; but is more explicit, and ensures the function is treated
    /// as a generator.
    ///
    ///     yield break;
    #[rust_to_ocaml(name = "Yield_break")]
    YieldBreak,
    /// Concurrent block. All the await expressions are awaited at the
    /// same time, similar to genva().
    ///
    /// We store the desugared form. In the below example, the list is:
    /// [('__tmp$1', f()), (__tmp$2, g()), (None, h())]
    /// and the block assigns the temporary variables back to the locals.
    /// { $foo = __tmp$1; $bar = __tmp$2; }
    ///
    ///     concurrent {
    ///       $foo = await f();
    ///       $bar = await g();
    ///       await h();
    ///     }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Awaitall(
        &'a (
            &'a [(&'a Lid<'a>, &'a Expr<'a, Ex, En>)],
            &'a Block<'a, Ex, En>,
        ),
    ),
    /// Concurrent block. All the await expressions are awaited at the
    /// same time, similar to genva().
    ///
    ///     concurrent {
    ///       $foo = await f();
    ///       $bar = await g();
    ///       await h();
    ///     }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Concurrent(&'a Block<'a, Ex, En>),
    /// If statement.
    ///
    ///     if ($foo) { ... } else { ... }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    If(
        &'a (
            &'a Expr<'a, Ex, En>,
            &'a Block<'a, Ex, En>,
            &'a Block<'a, Ex, En>,
        ),
    ),
    /// Do-while loop.
    ///
    ///     do {
    ///       bar();
    ///     } while($foo)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Do(&'a (&'a Block<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
    /// While loop.
    ///
    ///     while ($foo) {
    ///       bar();
    ///     }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    While(&'a (&'a Expr<'a, Ex, En>, &'a Block<'a, Ex, En>)),
    /// Initialize a value that is automatically disposed of.
    ///
    ///     using $foo = bar(); // disposed at the end of the function
    ///     using ($foo = bar(), $baz = quux()) {} // disposed after the block
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Using(&'a UsingStmt<'a, Ex, En>),
    /// For loop. The initializer and increment parts can include
    /// multiple comma-separated statements. The termination condition is
    /// optional.
    ///
    ///     for ($i = 0; $i < 100; $i++) { ... }
    ///     for ($x = 0, $y = 0; ; $x++, $y++) { ... }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    For(
        &'a (
            &'a [&'a Expr<'a, Ex, En>],
            Option<&'a Expr<'a, Ex, En>>,
            &'a [&'a Expr<'a, Ex, En>],
            &'a Block<'a, Ex, En>,
        ),
    ),
    /// Switch statement.
    ///
    ///     switch ($foo) {
    ///       case X:
    ///         bar();
    ///         break;
    ///       default:
    ///         baz();
    ///         break;
    ///     }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Switch(
        &'a (
            &'a Expr<'a, Ex, En>,
            &'a [Case<'a, Ex, En>],
            Option<DefaultCase<'a, Ex, En>>,
        ),
    ),
    /// Match statement.
    ///
    ///     match ($x) {
    ///       _: FooClass => {
    ///         foo($x);
    ///       }
    ///       _ => {
    ///         bar();
    ///       }
    ///     }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Match(&'a StmtMatch<'a, Ex, En>),
    /// For-each loop.
    ///
    ///     foreach ($items as $item) { ... }
    ///     foreach ($items as $key => value) { ... }
    ///     foreach ($items await as $item) { ... } // AsyncIterator<_>
    ///     foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Foreach(
        &'a (
            &'a Expr<'a, Ex, En>,
            AsExpr<'a, Ex, En>,
            &'a Block<'a, Ex, En>,
        ),
    ),
    /// Try statement, with catch blocks and a finally block.
    ///
    ///     try {
    ///       foo();
    ///     } catch (SomeException $e) {
    ///       bar();
    ///     } finally {
    ///       baz();
    ///     }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Try(
        &'a (
            &'a Block<'a, Ex, En>,
            &'a [&'a Catch<'a, Ex, En>],
            &'a FinallyBlock<'a, Ex, En>,
        ),
    ),
    /// Declare a local variable with the given type and optional initial value
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Declare_local")]
    #[rust_to_ocaml(inline_tuple)]
    DeclareLocal(&'a (&'a Lid<'a>, &'a Hint<'a>, Option<&'a Expr<'a, Ex, En>>)),
    /// Block, a list of statements in curly braces.
    ///
    ///     { $foo = 42; }
    /// If present, the optional list of identifiers are those that are scoped to this
    /// Block, they will be unset upon exit to the block.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Block(&'a (Option<&'a [&'a Lid<'a>]>, &'a Block<'a, Ex, En>)),
    /// The mode tag at the beginning of a file.
    /// TODO: this really belongs in def.
    ///
    ///     <?hh
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    Markup(&'a Pstring<'a>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Stmt_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Stmt_<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "us_")]
#[repr(C)]
pub struct UsingStmt<'a, Ex, En> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub exprs: (&'a Pos<'a>, &'a [&'a Expr<'a, Ex, En>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub block: &'a Block<'a, Ex, En>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for UsingStmt<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(UsingStmt<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum AsExpr<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "As_v")]
    AsV(&'a Expr<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "As_kv")]
    #[rust_to_ocaml(inline_tuple)]
    AsKv(&'a (&'a Expr<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Await_as_v")]
    #[rust_to_ocaml(inline_tuple)]
    AwaitAsV(&'a (&'a Pos<'a>, &'a Expr<'a, Ex, En>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Await_as_kv")]
    #[rust_to_ocaml(inline_tuple)]
    AwaitAsKv(&'a (&'a Pos<'a>, &'a Expr<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for AsExpr<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(AsExpr<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Block<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a [&'a Stmt<'a, Ex, En>],
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Block<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Block<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct FinallyBlock<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a [&'a Stmt<'a, Ex, En>],
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for FinallyBlock<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(FinallyBlock<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "sm_")]
#[repr(C)]
pub struct StmtMatch<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: &'a Expr<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub arms: &'a [&'a StmtMatchArm<'a, Ex, En>],
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for StmtMatch<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(StmtMatch<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "sma_")]
#[repr(C)]
pub struct StmtMatchArm<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pat: Pattern<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub body: &'a Block<'a, Ex, En>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for StmtMatchArm<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(StmtMatchArm<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Pattern<'a> {
    /// Variable patterns
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    PVar(&'a PatVar<'a>),
    /// Refinement patterns
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    PRefinement(&'a PatRefinement<'a>),
}
impl<'a> TrivialDrop for Pattern<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Pattern<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "pv_")]
#[repr(C)]
pub struct PatVar<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub pos: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Option<&'a Lid<'a>>,
}
impl<'a> TrivialDrop for PatVar<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PatVar<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "pr_")]
#[repr(C)]
pub struct PatRefinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub pos: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Option<&'a Lid<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub hint: &'a Hint<'a>,
}
impl<'a> TrivialDrop for PatRefinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PatRefinement<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ClassId<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub ClassId_<'a, Ex, En>,
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ClassId<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ClassId<'arena, Ex, En>);

/// Class ID, used in things like instantiation and static property access.
#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ClassId_<'a, Ex, En> {
    /// The class ID of the parent of the lexically scoped class.
    ///
    /// In a trait, it is the parent class ID of the using class.
    ///
    ///     parent::some_meth()
    ///     parent::$prop = 1;
    ///     new parent();
    CIparent,
    /// The class ID of the lexically scoped class.
    ///
    /// In a trait, it is the class ID of the using class.
    ///
    ///     self::some_meth()
    ///     self::$prop = 1;
    ///     new self();
    CIself,
    /// The class ID of the late static bound class.
    ///
    /// https://www.php.net/manual/en/language.oop5.late-static-bindings.php
    ///
    /// In a trait, it is the late static bound class ID of the using class.
    ///
    ///     static::some_meth()
    ///     static::$prop = 1;
    ///     new static();
    CIstatic,
    /// Dynamic class name.
    ///
    /// TODO: Syntactically this can only be an Lvar/This/Lplaceholder.
    /// We should use lid rather than expr.
    ///
    ///     // Assume $d has type dynamic.
    ///     $d::some_meth();
    ///     $d::$prop = 1;
    ///     new $d();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CIexpr(&'a Expr<'a, Ex, En>),
    /// Explicit class name. This is the common case.
    ///
    ///     Foo::some_meth()
    ///     Foo::$prop = 1;
    ///     new Foo();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CI(&'a ClassName<'a>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ClassId_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ClassId_<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Expr<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Expr_<'a, Ex, En>,
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Expr<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Expr<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>"))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum CollectionTarg<'a, Ex> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CollectionTV(&'a Targ<'a, Ex>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    CollectionTKV(&'a (&'a Targ<'a, Ex>, &'a Targ<'a, Ex>)),
}
impl<'a, Ex: TrivialDrop> TrivialDrop for CollectionTarg<'a, Ex> {}
arena_deserializer::impl_deserialize_in_arena!(CollectionTarg<'arena, Ex>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum FunctionPtrId<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "FP_id")]
    FPId(&'a Sid<'a>),
    /// An expression tree literal consists of a hint, splices, and
    ///  expressions. Consider this example:
    ///
    ///     Foo`1 + ${$x} + ${bar()}`
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "FP_class_const")]
    #[rust_to_ocaml(inline_tuple)]
    FPClassConst(&'a (&'a ClassId<'a, Ex, En>, &'a Pstring<'a>)),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for FunctionPtrId<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(FunctionPtrId<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "et_")]
#[repr(C)]
pub struct ExpressionTree<'a, Ex, En> {
    /// The hint before the backtick, so Foo in this example.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub class: &'a ClassName<'a>,
    /// The values spliced into expression tree at runtime are assigned
    /// to temporaries.
    ///
    ///     $0tmp1 = $x; $0tmp2 = bar();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub splices: &'a [&'a Stmt<'a, Ex, En>],
    /// The list of global functions and static methods assigned to
    /// temporaries.
    ///
    ///     $0fp1 = foo<>;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub function_pointers: &'a [&'a Stmt<'a, Ex, En>],
    /// The expression that's executed at runtime.
    ///
    ///     Foo::makeTree($v ==> $v->visitBinOp(...))
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub runtime_expr: &'a Expr<'a, Ex, En>,
    /// Position of the first $$ in a splice that refers
    /// to a variable outside the Expression Tree
    ///
    ///     $x |> Code`${ $$ }` // Pos of the $$
    ///     Code`${ $x |> foo($$) }` // None
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub dollardollar_pos: Option<&'a Pos<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ExpressionTree<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ExpressionTree<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct As_<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: &'a Expr<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub hint: &'a Hint<'a>,
    pub is_nullable: bool,
    pub enforce_deep: bool,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for As_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(As_<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Expr_<'a, Ex, En> {
    /// Null literal.
    ///
    ///     null
    Null,
    /// Boolean literal.
    ///
    ///     true
    True,
    /// Boolean literal.
    ///
    ///     false
    False,
    /// Shape literal.
    ///
    ///     shape('x' => 1, 'y' => 2)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Shape(&'a [(ast_defs::ShapeFieldName<'a>, &'a Expr<'a, Ex, En>)]),
    /// Collection literal for indexable structures.
    ///
    ///     Vector {1, 2}
    ///     ImmVector {}
    ///     Set<string> {'foo', 'bar'}
    ///     vec[1, 2]
    ///     keyset[]
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    ValCollection(
        &'a (
            (&'a Pos<'a>, oxidized::aast_defs::VcKind),
            Option<&'a Targ<'a, Ex>>,
            &'a [&'a Expr<'a, Ex, En>],
        ),
    ),
    /// Collection literal for key-value structures.
    ///
    ///     dict['x' => 1, 'y' => 2]
    ///     Map<int, string> {}
    ///     ImmMap {}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    KeyValCollection(
        &'a (
            (&'a Pos<'a>, oxidized::aast_defs::KvcKind),
            Option<&'a (&'a Targ<'a, Ex>, &'a Targ<'a, Ex>)>,
            &'a [&'a Field<'a, Ex, En>],
        ),
    ),
    /// The local variable representing the current class instance.
    ///
    ///     $this
    This,
    /// The empty expression.
    ///
    ///     list(, $y) = vec[1, 2] // Omitted is the first expression inside list()
    Omitted,
    /// Invalid expression marker generated during elaboration / validation phases
    ///
    ///     class MyFoo {
    ///       const int BAR = calls_are_invalid_here();
    ///     }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Invalid(Option<&'a Expr<'a, Ex, En>>),
    /// An identifier. Used for method names and global constants.
    ///
    ///     SOME_CONST
    ///     $x->foo() // id: "foo"
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Id(&'a Sid<'a>),
    /// Local variable.
    ///
    ///     $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Lvar(&'a Lid<'a>),
    /// The extra variable in a pipe expression.
    ///
    ///     $$
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Dollardollar(&'a Lid<'a>),
    /// Clone expression.
    ///
    ///     clone $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Clone(&'a Expr<'a, Ex, En>),
    /// Array indexing.
    ///
    ///     $foo[]
    ///     $foo[$bar]
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Array_get")]
    #[rust_to_ocaml(inline_tuple)]
    ArrayGet(&'a (&'a Expr<'a, Ex, En>, Option<&'a Expr<'a, Ex, En>>)),
    /// Instance property or method access.
    ///
    ///     $foo->bar      // OG_nullthrows, Is_prop: access named property
    ///     ($foo->bar)()  // OG_nullthrows, Is_prop: call lambda stored in named property
    ///     $foo?->bar     // OG_nullsafe,   Is_prop
    ///     ($foo?->bar)() // OG_nullsafe,   Is_prop
    ///
    ///     $foo->bar()    // OG_nullthrows, Is_method: call named method
    ///     $foo->$bar()   // OG_nullthrows, Is_method: dynamic call, method name stored in local $bar
    ///     $foo?->bar()   // OG_nullsafe,   Is_method
    ///     $foo?->$bar()  // OG_nullsafe,   Is_method
    ///
    /// prop_or_method is:
    ///   - Is_prop for property access
    ///   - Is_method for method call, only possible when the node is the receiver in a Call node.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Obj_get")]
    #[rust_to_ocaml(inline_tuple)]
    ObjGet(
        &'a (
            &'a Expr<'a, Ex, En>,
            &'a Expr<'a, Ex, En>,
            &'a oxidized::aast_defs::OgNullFlavor,
            &'a oxidized::aast_defs::PropOrMethod,
        ),
    ),
    /// Static property or dynamic method access. The rhs of the :: begins
    /// with $ or is some non-name expression appearing within braces {}.
    ///
    ///     Foo::$bar               // Is_prop: access named static property
    ///     Foo::{$bar}             // Is_prop
    ///     (Foo::$bar)();          // Is_prop: call lambda stored in static property Foo::$bar
    ///     $classname::$bar        // Is_prop
    ///
    ///     Foo::$bar();            // Is_method: dynamic call, method name stored in local $bar
    ///     Foo::{$bar}();          // Is_method
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Class_get")]
    #[rust_to_ocaml(inline_tuple)]
    ClassGet(
        &'a (
            &'a ClassId<'a, Ex, En>,
            ClassGetExpr<'a, Ex, En>,
            &'a oxidized::aast_defs::PropOrMethod,
        ),
    ),
    /// Class constant or static method call. As a standalone expression,
    /// this is a class constant. Inside a Call node, this is a static
    /// method call. The rhs of the :: does not begin with $ or is a name
    /// appearing within braces {}.
    ///
    /// This is not ambiguous, because constants are not allowed to
    /// contain functions.
    ///
    ///     Foo::some_const            // Const
    ///     Foo::{another_const}       // Const: braces are elided
    ///     Foo::class                 // Const: fully qualified class name of Foo
    ///     Foo::staticMeth()          // Call
    ///     $classname::staticMeth()   // Call
    ///
    /// This syntax is used for both static and instance methods when
    /// calling the implementation on the superclass.
    ///
    ///     parent::someStaticMeth()
    ///     parent::someInstanceMeth()
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Class_const")]
    #[rust_to_ocaml(inline_tuple)]
    ClassConst(&'a (&'a ClassId<'a, Ex, En>, &'a Pstring<'a>)),
    /// Function or method call.
    ///
    ///     foo()
    ///     $x()
    ///     foo<int>(1, 2, ...$rest)
    ///     $x->foo()
    ///     bar(inout $x);
    ///     foobar(inout $x[0])
    ///
    ///     async { return 1; }
    ///     // lowered to:
    ///     (async () ==> { return 1; })()
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Call(&'a CallExpr<'a, Ex, En>),
    /// A reference to a function or method.
    ///
    ///     foo_fun<>
    ///     FooCls::meth<int>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    FunctionPointer(&'a (FunctionPtrId<'a, Ex, En>, &'a [&'a Targ<'a, Ex>])),
    /// Integer literal.
    ///
    ///     42
    ///     0123 // octal
    ///     0xBEEF // hexadecimal
    ///     0b11111111 // binary
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Int(&'a str),
    /// Float literal.
    ///
    ///     1.0
    ///     1.2e3
    ///     7E-10
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Float(&'a str),
    /// String literal.
    ///
    ///     "foo"
    ///     'foo'
    ///
    ///     <<<DOC
    ///     foo
    ///     DOC
    ///
    ///     <<<'DOC'
    ///     foo
    ///     DOC
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    String(&'a bstr::BStr),
    /// Interpolated string literal.
    ///
    ///     "hello $foo $bar"
    ///
    ///     <<<DOC
    ///     hello $foo $bar
    ///     DOC
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    String2(&'a [&'a Expr<'a, Ex, En>]),
    /// Prefixed string literal. Only used for regular expressions.
    ///
    ///     re"foo"
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    PrefixedString(&'a (&'a str, &'a Expr<'a, Ex, En>)),
    /// Yield expression. The enclosing function should have an Iterator
    /// return type.
    ///
    ///     yield $foo // enclosing function returns an Iterator
    ///     yield $foo => $bar // enclosing function returns a KeyedIterator
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Yield(&'a Afield<'a, Ex, En>),
    /// Await expression.
    ///
    ///     await $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Await(&'a Expr<'a, Ex, En>),
    /// Readonly expression.
    ///
    ///     readonly $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ReadonlyExpr(&'a Expr<'a, Ex, En>),
    /// Tuple expression.
    ///
    ///     tuple("a", 1, $foo)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tuple(&'a [&'a Expr<'a, Ex, En>]),
    /// List expression, only used in destructuring. Allows any arbitrary
    /// lvalue as a subexpression. May also nest.
    ///
    ///     list($x, $y) = vec[1, 2];
    ///     list(, $y) = vec[1, 2]; // skipping items
    ///     list(list($x)) = vec[vec[1]]; // nesting
    ///     list($v[0], $x[], $y->foo) = $blah;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    List(&'a [&'a Expr<'a, Ex, En>]),
    /// Cast expression, converting a value to a different type. Only
    /// primitive types are supported in the hint position.
    ///
    ///     (int)$foo
    ///     (string)$foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Cast(&'a (&'a Hint<'a>, &'a Expr<'a, Ex, En>)),
    /// Unary operator.
    ///
    ///     !$foo
    ///     -$foo
    ///     +$foo
    ///     $foo++
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Unop(&'a (oxidized::ast_defs::Uop, &'a Expr<'a, Ex, En>)),
    /// Binary operator.
    ///
    ///     $foo + $bar
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Binop(&'a Binop<'a, Ex, En>),
    /// Pipe expression. The lid is the ID of the $$ that is implicitly
    /// declared by this pipe.
    ///
    /// See also Dollardollar.
    ///
    ///     foo() |> bar(1, $$) // equivalent: bar(1, foo())
    ///
    /// $$ is not required on the RHS of pipe expressions, but it's
    /// pretty pointless to use pipes without $$.
    ///
    ///     foo() |> bar(); // equivalent: foo(); bar();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Pipe(&'a (&'a Lid<'a>, &'a Expr<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
    /// Ternary operator, or elvis operator.
    ///
    ///     $foo ? $bar : $baz // ternary
    ///     $foo ?: $baz // elvis
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Eif(
        &'a (
            &'a Expr<'a, Ex, En>,
            Option<&'a Expr<'a, Ex, En>>,
            &'a Expr<'a, Ex, En>,
        ),
    ),
    /// Is operator.
    ///
    ///     $foo is SomeType
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Is(&'a (&'a Expr<'a, Ex, En>, &'a Hint<'a>)),
    /// As operator.
    ///
    ///     $foo as int
    ///     $foo ?as int
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    As(&'a As_<'a, Ex, En>),
    /// Upcast operator.
    ///
    ///     $foo : int
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Upcast(&'a (&'a Expr<'a, Ex, En>, &'a Hint<'a>)),
    /// Instantiation.
    ///
    ///     new Foo(1, 2);
    ///     new Foo<int, T>();
    ///     new Foo('blah', ...$rest);
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    New(
        &'a (
            &'a ClassId<'a, Ex, En>,
            &'a [&'a Targ<'a, Ex>],
            &'a [&'a Expr<'a, Ex, En>],
            Option<&'a Expr<'a, Ex, En>>,
            Ex,
        ),
    ),
    /// PHP-style lambda. Does not capture variables unless explicitly
    /// specified.
    ///
    /// Mnemonic: 'expanded lambda', since we can desugar Lfun to Efun.
    ///
    ///     function($x) { return $x; }
    ///     function(int $x): int { return $x; }
    ///     function($x) use ($y) { return $y; }
    ///     function($x): int use ($y, $z) { return $x + $y + $z; }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Efun(&'a Efun<'a, Ex, En>),
    /// Hack lambda. Captures variables automatically.
    ///
    ///     $x ==> $x
    ///     (int $x): int ==> $x + $other
    ///     ($x, $y) ==> { return $x + $y; }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Lfun(&'a (&'a Fun_<'a, Ex, En>, &'a [&'a CaptureLid<'a, Ex>])),
    /// XHP expression. May contain interpolated expressions.
    ///
    ///     <foo x="hello" y={$foo}>hello {$bar}</foo>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Xml(
        &'a (
            &'a ClassName<'a>,
            &'a [XhpAttribute<'a, Ex, En>],
            &'a [&'a Expr<'a, Ex, En>],
        ),
    ),
    /// Include or require expression.
    ///
    ///     require('foo.php')
    ///     require_once('foo.php')
    ///     include('foo.php')
    ///     include_once('foo.php')
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Import(&'a (oxidized::aast_defs::ImportFlavor, &'a Expr<'a, Ex, En>)),
    /// Collection literal.
    ///
    /// TODO: T38184446 this is redundant with ValCollection/KeyValCollection.
    ///
    ///     Vector {}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Collection(
        &'a (
            &'a ClassName<'a>,
            Option<CollectionTarg<'a, Ex>>,
            &'a [Afield<'a, Ex, En>],
        ),
    ),
    /// Expression tree literal. Expression trees are not evaluated at
    /// runtime, but desugared to an expression representing the code.
    ///
    ///     Foo`1 + bar()`
    ///     Foo`(() ==> { while(true) {} })()` // not an infinite loop at runtime
    ///
    /// Splices are evaluated as normal Hack code. The following two expression trees
    /// are equivalent. See also `ET_Splice`.
    ///
    ///     Foo`1 + ${do_stuff()}`
    ///
    ///     $x = do_stuff();
    ///     Foo`1 + ${$x}`
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ExpressionTree(&'a ExpressionTree<'a, Ex, En>),
    /// Placeholder local variable.
    ///
    ///     $_
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    Lplaceholder(&'a Pos<'a>),
    /// Instance method reference that can be called with an instance.
    ///
    ///     meth_caller(FooClass::class, 'some_meth')
    ///
    /// These examples are equivalent to:
    ///
    ///     (FooClass $f, ...$args) ==> $f->some_meth(...$args)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Method_caller")]
    #[rust_to_ocaml(inline_tuple)]
    MethodCaller(&'a (&'a ClassName<'a>, &'a Pstring<'a>)),
    /// Pair literal.
    ///
    ///     Pair {$foo, $bar}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Pair(
        &'a (
            Option<&'a (&'a Targ<'a, Ex>, &'a Targ<'a, Ex>)>,
            &'a Expr<'a, Ex, En>,
            &'a Expr<'a, Ex, En>,
        ),
    ),
    /// Expression tree splice expression. Only valid inside an
    /// expression tree literal (backticks). See also `ExpressionTree`.
    ///
    ///     ${$foo}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "ET_Splice")]
    ETSplice(&'a Expr<'a, Ex, En>),
    /// Label used for enum classes.
    ///
    ///     enum_name#label_name or #label_name
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    EnumClassLabel(&'a (Option<&'a ClassName<'a>>, &'a str)),
    /// Annotation used to record failure in subtyping or coercion of an
    /// expression and calls to [unsafe_cast] or [enforced_cast].
    ///
    /// The [hole_source] indicates whether this came from an
    /// explicit call to [unsafe_cast] or [enforced_cast] or was
    /// generated during typing.
    ///
    /// Given a call to [unsafe_cast]:
    /// ```
    /// function f(int $x): void { /* ... */ }
    ///
    /// function g(float $x): void {
    ///    f(unsafe_cast<float,int>($x));
    /// }
    /// ```
    /// After typing, this is represented by the following TAST fragment
    /// ```
    /// Call
    ///   ( ( (..., function(int $x): void), Id (..., "\f"))
    ///   , []
    ///   , [ ( (..., int)
    ///       , Hole
    ///           ( ((..., float), Lvar (..., $x))
    ///           , float
    ///           , int
    ///           , UnsafeCast
    ///           )
    ///       )
    ///     ]
    ///   , None
    ///   )
    /// ```
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Hole(&'a (&'a Expr<'a, Ex, En>, Ex, Ex, HoleSource<'a>)),
    /// Expression used to check whether a package exists.
    ///
    ///     package package-name
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Package(&'a Sid<'a>),
    /// Get the name of a class
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Nameof(&'a ClassId<'a, Ex, En>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Expr_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Expr_<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum HoleSource<'a> {
    Typing,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    UnsafeCast(&'a [&'a Hint<'a>]),
    UnsafeNonnullCast,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    EnforcedCast(&'a [&'a Hint<'a>]),
}
impl<'a> TrivialDrop for HoleSource<'a> {}
arena_deserializer::impl_deserialize_in_arena!(HoleSource<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Binop<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub bop: ast_defs::Bop<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub lhs: &'a Expr<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub rhs: &'a Expr<'a, Ex, En>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Binop<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Binop<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ClassGetExpr<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CGstring(&'a Pstring<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CGexpr(&'a Expr<'a, Ex, En>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ClassGetExpr<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ClassGetExpr<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Case<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Expr<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Block<'a, Ex, En>,
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Case<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Case<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct DefaultCase<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Block<'a, Ex, En>,
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for DefaultCase<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(DefaultCase<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum GenCase<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Case(&'a Case<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Default(&'a DefaultCase<'a, Ex, En>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for GenCase<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(GenCase<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Catch<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a ClassName<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Lid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Block<'a, Ex, En>,
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Catch<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Catch<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Field<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Expr<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Expr<'a, Ex, En>,
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Field<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Field<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Afield<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AFvalue(&'a Expr<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    AFkvalue(&'a (&'a Expr<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Afield<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Afield<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "xs_")]
#[repr(C)]
pub struct XhpSimple<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub name: &'a Pstring<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub type_: Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: &'a Expr<'a, Ex, En>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for XhpSimple<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(XhpSimple<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum XhpAttribute<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Xhp_simple")]
    XhpSimple(&'a XhpSimple<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Xhp_spread")]
    XhpSpread(&'a Expr<'a, Ex, En>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for XhpAttribute<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(XhpAttribute<'arena, Ex, En>);

pub use oxidized::aast_defs::IsVariadic;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "param_")]
#[repr(C)]
pub struct FunParam<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_hint: &'a TypeHint<'a, Ex>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub is_variadic: &'a oxidized::aast_defs::IsVariadic,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub pos: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: Option<&'a Expr<'a, Ex, En>>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub readonly: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub callconv: ast_defs::ParamKind<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub visibility: Option<oxidized::aast_defs::Visibility>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for FunParam<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(FunParam<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "f_")]
#[repr(C)]
pub struct Fun_<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub readonly_this: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    /// Whether the return value is readonly
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub readonly_ret: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub ret: &'a TypeHint<'a, Ex>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub params: &'a [&'a FunParam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub unsafe_ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub body: &'a FuncBody<'a, Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub fun_kind: oxidized::ast_defs::FunKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Fun_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Fun_<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>"))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct CaptureLid<'a, Ex>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Lid<'a>,
);
impl<'a, Ex: TrivialDrop> TrivialDrop for CaptureLid<'a, Ex> {}
arena_deserializer::impl_deserialize_in_arena!(CaptureLid<'arena, Ex>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "ef_")]
#[repr(C)]
pub struct Efun<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fun: &'a Fun_<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub use_: &'a [&'a CaptureLid<'a, Ex>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub closure_class_name: Option<&'a str>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Efun<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Efun<'arena, Ex, En>);

/// Naming has two phases and the annotation helps to indicate the phase.
/// In the first pass, it will perform naming on everything except for function
/// and method bodies and collect information needed. Then, another round of
/// naming is performed where function bodies are named.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct FuncBody<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fb_ast: &'a Block<'a, Ex, En>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for FuncBody<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(FuncBody<'arena, Ex, En>);

/// A type annotation is two things:
/// - the localized hint, or if the hint is missing, the inferred type
/// - The typehint associated to this expression if it exists
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>"))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct TypeHint<'a, Ex>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a TypeHint_<'a>,
);
impl<'a, Ex: TrivialDrop> TrivialDrop for TypeHint<'a, Ex> {}
arena_deserializer::impl_deserialize_in_arena!(TypeHint<'arena, Ex>);

/// Explicit type argument to function, constructor, or collection literal.
/// 'ex = unit in NAST
/// 'ex = Typing_defs.(locl ty) in TAST,
/// and is used to record inferred type arguments, with wildcard hint.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>"))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Targ<'a, Ex>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
);
impl<'a, Ex: TrivialDrop> TrivialDrop for Targ<'a, Ex> {}
arena_deserializer::impl_deserialize_in_arena!(Targ<'arena, Ex>);

#[rust_to_ocaml(and)]
pub type TypeHint_<'a> = Option<&'a Hint<'a>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct CallExpr<'a, Ex, En> {
    /// function
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub func: &'a Expr<'a, Ex, En>,
    /// explicit type annotations
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub targs: &'a [&'a Targ<'a, Ex>],
    /// positional args, plus their calling convention
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub args: &'a [(ast_defs::ParamKind<'a>, &'a Expr<'a, Ex, En>)],
    /// unpacked arg
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub unpacked_arg: Option<&'a Expr<'a, Ex, En>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for CallExpr<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(CallExpr<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "ua_")]
#[repr(C)]
pub struct UserAttribute<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    /// user attributes are restricted to scalar values
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub params: &'a [&'a Expr<'a, Ex, En>],
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for UserAttribute<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(UserAttribute<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "fa_")]
#[repr(C)]
pub struct FileAttribute<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for FileAttribute<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(FileAttribute<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "tp_")]
#[repr(C)]
pub struct Tparam<'a, Ex, En> {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub variance: oxidized::ast_defs::Variance,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub parameters: &'a [&'a Tparam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraints: &'a [(oxidized::ast_defs::ConstraintKind, &'a Hint<'a>)],
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub reified: oxidized::aast_defs::ReifyKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Tparam<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Tparam<'arena, Ex, En>);

pub use oxidized::aast_defs::EmitId;
pub use oxidized::aast_defs::RequireKind;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "c_")]
#[repr(C)]
pub struct Class_<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub kind: oxidized::ast_defs::ClassishKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a ClassName<'a>,
    /// The type parameters of a class A<T> (T is the parameter)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub tparams: &'a [&'a Tparam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub extends: &'a [&'a ClassHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub uses: &'a [&'a TraitHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub xhp_attr_uses: &'a [&'a XhpAttrHint<'a>],
    /// Categories are the equivalent of interfaces for XHP classes.
    /// These are either listed after the `implements` keyword, or as such
    /// within the XHP class:
    ///
    /// category cat1, cat2, cat3;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub xhp_category: Option<&'a (&'a Pos<'a>, &'a [&'a Pstring<'a>])>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub reqs: &'a [&'a ClassReq<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub implements: &'a [&'a ClassHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub consts: &'a [&'a ClassConst<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub typeconsts: &'a [&'a ClassTypeconstDef<'a, Ex, En>],
    /// a.k.a properties
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub vars: &'a [&'a ClassVar<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub methods: &'a [&'a Method_<'a, Ex, En>],
    /// XHP allows an XHP class to declare which types are allowed
    /// as chidren using the `children` keyword.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_children: &'a [(&'a Pos<'a>, XhpChild<'a>)],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub xhp_attrs: &'a [&'a XhpAttr<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub docs_url: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub enum_: Option<&'a Enum_<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub emit_id: Option<&'a oxidized::aast_defs::EmitId>,
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<Sid<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Class_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Class_<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ClassReq<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a ClassHint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a oxidized::aast_defs::RequireKind,
);
impl<'a> TrivialDrop for ClassReq<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassReq<'arena>);

#[rust_to_ocaml(and)]
pub type ClassHint<'a> = Hint<'a>;

#[rust_to_ocaml(and)]
pub type TraitHint<'a> = Hint<'a>;

#[rust_to_ocaml(and)]
pub type XhpAttrHint<'a> = Hint<'a>;

pub use oxidized::aast_defs::XhpAttrTag;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct XhpAttr<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a TypeHint<'a, Ex>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a ClassVar<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  Option<&'a oxidized::aast_defs::XhpAttrTag>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  Option<&'a (&'a Pos<'a>, &'a [&'a Expr<'a, Ex, En>])>,
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for XhpAttr<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(XhpAttr<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ClassConstKind<'a, Ex, En> {
    /// CCAbstract represents the states
    ///     abstract const int X;
    ///     abstract const int Y = 4;
    /// The expr option is a default value
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CCAbstract(Option<&'a Expr<'a, Ex, En>>),
    /// CCConcrete represents
    ///     const int Z = 4;
    /// The expr is the value of the constant. It is not optional
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CCConcrete(&'a Expr<'a, Ex, En>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ClassConstKind<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ClassConstKind<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "cc_")]
#[repr(C)]
pub struct ClassConst<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: ClassConstKind<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ClassConst<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ClassConst<'arena, Ex, En>);

/// This represents a type const definition. If a type const is abstract then
/// then the type hint acts as a constraint. Any concrete definition of the
/// type const must satisfy the constraint.
///
/// If the type const is not abstract then a type must be specified.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "c_atc_")]
#[repr(C)]
pub struct ClassAbstractTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub as_constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub super_constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub default: Option<&'a Hint<'a>>,
}
impl<'a> TrivialDrop for ClassAbstractTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassAbstractTypeconst<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ClassConcreteTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub c_tc_type: &'a Hint<'a>,
}
impl<'a> TrivialDrop for ClassConcreteTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassConcreteTypeconst<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ClassTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TCAbstract(&'a ClassAbstractTypeconst<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TCConcrete(&'a ClassConcreteTypeconst<'a>),
}
impl<'a> TrivialDrop for ClassTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassTypeconst<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "c_tconst_")]
#[repr(C)]
pub struct ClassTypeconstDef<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: ClassTypeconst<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub is_ctx: bool,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ClassTypeconstDef<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ClassTypeconstDef<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "xai_")]
#[repr(C)]
pub struct XhpAttrInfo<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub like: Option<&'a Pos<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tag: Option<&'a oxidized::aast_defs::XhpAttrTag>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub enum_values: &'a [ast_defs::XhpEnumValue<'a>],
}
impl<'a> TrivialDrop for XhpAttrInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(XhpAttrInfo<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "cv_")]
#[repr(C)]
pub struct ClassVar<'a, Ex, En> {
    pub final_: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attr: Option<&'a XhpAttrInfo<'a>>,
    pub abstract_: bool,
    pub readonly: bool,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub visibility: oxidized::aast_defs::Visibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub type_: &'a TypeHint<'a, Ex>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: Option<&'a Expr<'a, Ex, En>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub is_static: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ClassVar<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ClassVar<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "m_")]
#[repr(C)]
pub struct Method_<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub readonly_this: bool,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub visibility: oxidized::aast_defs::Visibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub params: &'a [&'a FunParam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub unsafe_ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub body: &'a FuncBody<'a, Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub fun_kind: oxidized::ast_defs::FunKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub readonly_ret: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub ret: &'a TypeHint<'a, Ex>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Method_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Method_<'arena, Ex, En>);

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
pub type Nsenv<'a> = namespace_env::Env<'a>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "t_")]
#[repr(C)]
pub struct Typedef<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub as_constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub super_constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub kind: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, En>],
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: oxidized::file_info::Mode,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub vis: oxidized::aast_defs::TypedefVisibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub emit_id: Option<&'a oxidized::aast_defs::EmitId>,
    pub is_ctx: bool,
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<Sid<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub docs_url: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Typedef<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Typedef<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "cst_")]
#[repr(C)]
pub struct Gconst<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: oxidized::file_info::Mode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub value: &'a Expr<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub emit_id: Option<&'a oxidized::aast_defs::EmitId>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<Sid<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Gconst<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Gconst<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "fd_")]
#[repr(C)]
pub struct FunDef<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, En>],
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: oxidized::file_info::Mode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fun: &'a Fun_<'a, Ex, En>,
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<Sid<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for FunDef<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(FunDef<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "md_")]
#[repr(C)]
pub struct ModuleDef<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub name: ast_defs::Id<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a UserAttributes<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: &'a Pos<'a>,
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: oxidized::file_info::Mode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub exports: Option<&'a [MdNameKind<'a>]>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub imports: Option<&'a [MdNameKind<'a>]>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ModuleDef<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ModuleDef<'arena, Ex, En>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum MdNameKind<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    MDNameGlobal(&'a Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    MDNamePrefix(&'a Sid<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    MDNameExact(&'a Sid<'a>),
}
impl<'a> TrivialDrop for MdNameKind<'a> {}
arena_deserializer::impl_deserialize_in_arena!(MdNameKind<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Def<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Fun(&'a FunDef<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Class(&'a Class_<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Stmt(&'a Stmt<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Typedef(&'a Typedef<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Constant(&'a Gconst<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Namespace(&'a (Sid<'a>, &'a [Def<'a, Ex, En>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    NamespaceUse(&'a [(&'a oxidized::aast_defs::NsKind, Sid<'a>, Sid<'a>)]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    SetNamespaceEnv(&'a Nsenv<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FileAttributes(&'a FileAttribute<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Module(&'a ModuleDef<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    SetModule(&'a Sid<'a>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Def<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Def<'arena, Ex, En>);

pub use oxidized::aast_defs::NsKind;

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
pub type DocComment<'a> = ast_defs::Pstring<'a>;

pub use oxidized::aast_defs::ImportFlavor;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum XhpChild<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ChildName(&'a Sid<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ChildList(&'a [XhpChild<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    ChildUnary(&'a (XhpChild<'a>, oxidized::aast_defs::XhpChildOp)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    ChildBinary(&'a (XhpChild<'a>, XhpChild<'a>)),
}
impl<'a> TrivialDrop for XhpChild<'a> {}
arena_deserializer::impl_deserialize_in_arena!(XhpChild<'arena>);

pub use oxidized::aast_defs::XhpChildOp;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Hint<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint_<'a>,
);
impl<'a> TrivialDrop for Hint<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Hint<'arena>);

#[rust_to_ocaml(and)]
pub type VariadicHint<'a> = Option<&'a Hint<'a>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct UserAttributes<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a [&'a UserAttribute<'a, Ex, En>],
);
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for UserAttributes<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(UserAttributes<'arena, Ex, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Contexts<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a [&'a Context<'a>],
);
impl<'a> TrivialDrop for Contexts<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Contexts<'arena>);

#[rust_to_ocaml(and)]
pub type Context<'a> = Hint<'a>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[rust_to_ocaml(prefix = "hfparam_")]
#[repr(C)]
pub struct HfParamInfo<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: ast_defs::ParamKind<'a>,
    pub readonlyness: Option<oxidized::ast_defs::ReadonlyKind>,
    pub optional: Option<oxidized::ast_defs::OptionalKind>,
}
impl<'a> TrivialDrop for HfParamInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(HfParamInfo<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "hf_")]
#[repr(C)]
pub struct HintFun<'a> {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub is_readonly: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub param_tys: &'a [&'a Hint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub param_info: &'a [Option<&'a HfParamInfo<'a>>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub variadic_ty: &'a VariadicHint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub return_ty: &'a Hint<'a>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub is_readonly_return: Option<oxidized::ast_defs::ReadonlyKind>,
}
impl<'a> TrivialDrop for HintFun<'a> {}
arena_deserializer::impl_deserialize_in_arena!(HintFun<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Hint_<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hprim(&'a oxidized::aast_defs::Tprim),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Happly(&'a (&'a ClassName<'a>, &'a [&'a Hint<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hoption(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hlike(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hfun(&'a HintFun<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Htuple(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Hclass_args")]
    HclassArgs(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hshape(&'a NastShapeInfo<'a>),
    /// Accessing a type constant. Type constants are accessed like normal
    /// class constants, but in type positions.
    ///
    /// SomeClass::TFoo
    /// self::TFoo
    /// this::TFoo
    ///
    /// Type constants can be also be chained, hence the list as the second
    /// argument:
    ///
    /// SomeClass::TFoo::TBar // Haccess (Happly "SomeClass", ["TFoo", "TBar"])
    ///
    /// When using contexts, the receiver may be a variable rather than a
    /// specific type:
    ///
    /// function uses_const_ctx(SomeClassWithConstant $t)[$t::C]: void {}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Haccess(&'a (&'a Hint<'a>, &'a [Sid<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hsoft(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Hrefinement(&'a (&'a Hint<'a>, &'a [Refinement<'a>])),
    Hmixed,
    Hwildcard,
    Hnonnull,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Habstr(&'a (&'a str, &'a [&'a Hint<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Hvec_or_dict")]
    #[rust_to_ocaml(inline_tuple)]
    HvecOrDict(&'a (Option<&'a Hint<'a>>, &'a Hint<'a>)),
    Hthis,
    Hdynamic,
    Hnothing,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hunion(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hintersection(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Hfun_context")]
    HfunContext(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hvar(&'a str),
}
impl<'a> TrivialDrop for Hint_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Hint_<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Refinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rctx(&'a (Sid<'a>, CtxRefinement<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rtype(&'a (Sid<'a>, TypeRefinement<'a>)),
}
impl<'a> TrivialDrop for Refinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Refinement<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum TypeRefinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TRexact(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TRloose(&'a TypeRefinementBounds<'a>),
}
impl<'a> TrivialDrop for TypeRefinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypeRefinement<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "tr_")]
#[repr(C)]
pub struct TypeRefinementBounds<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lower: &'a [&'a Hint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub upper: &'a [&'a Hint<'a>],
}
impl<'a> TrivialDrop for TypeRefinementBounds<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypeRefinementBounds<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum CtxRefinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CRexact(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CRloose(&'a CtxRefinementBounds<'a>),
}
impl<'a> TrivialDrop for CtxRefinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CtxRefinement<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "cr_")]
#[repr(C)]
pub struct CtxRefinementBounds<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lower: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub upper: Option<&'a Hint<'a>>,
}
impl<'a> TrivialDrop for CtxRefinementBounds<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CtxRefinementBounds<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "sfi_")]
#[repr(C)]
pub struct ShapeFieldInfo<'a> {
    pub optional: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub hint: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub name: ast_defs::ShapeFieldName<'a>,
}
impl<'a> TrivialDrop for ShapeFieldInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeFieldInfo<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "nsi_")]
#[repr(C)]
pub struct NastShapeInfo<'a> {
    pub allows_unknown_fields: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub field_map: &'a [&'a ShapeFieldInfo<'a>],
}
impl<'a> TrivialDrop for NastShapeInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(NastShapeInfo<'arena>);

pub use oxidized::aast_defs::KvcKind;
pub use oxidized::aast_defs::VcKind;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "e_")]
#[repr(C)]
pub struct Enum_<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub base: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub includes: &'a [&'a Hint<'a>],
}
impl<'a> TrivialDrop for Enum_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Enum_<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = r#"deriving ((show { with_path = false }), eq, hash, ord, map,
    (transform ~restart:(`Disallow `Encode_as_result)),
    (visitors
       {
         variety = "iter";
         nude = true;
         visit_prefix = "on_";
         ancestors =
           ["Visitors_runtime.iter"; "Aast_defs_visitors_ancestors.iter"]
       }),
    (visitors
       {
         variety = "reduce";
         nude = true;
         visit_prefix = "on_";
         ancestors =
           ["Visitors_runtime.reduce"; "Aast_defs_visitors_ancestors.reduce"]
       }),
    (visitors
       {
         variety = "map";
         nude = true;
         visit_prefix = "on_";
         ancestors =
           ["Visitors_runtime.map"; "Aast_defs_visitors_ancestors.map"]
       }),
    (visitors
       {
         variety = "endo";
         nude = true;
         visit_prefix = "on_";
         ancestors =
           ["Visitors_runtime.endo"; "Aast_defs_visitors_ancestors.endo"]
       }))"#)]
#[repr(C)]
pub struct WhereConstraintHint<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
    pub oxidized::ast_defs::ConstraintKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
);
impl<'a> TrivialDrop for WhereConstraintHint<'a> {}
arena_deserializer::impl_deserialize_in_arena!(WhereConstraintHint<'arena>);
