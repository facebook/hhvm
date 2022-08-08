// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3456ffead8ab9aa6b96102923088f095>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use aast_defs::*;
use arena_trait::TrivialDrop;
pub use doc_comment::DocComment;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

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
#[repr(C, u8)]
pub enum Stmt_<'a, Ex, En> {
    /// Marker for a switch statement that falls through.
    ///
    /// // FALLTHROUGH
    Fallthrough,
    /// Standalone expression.
    ///
    /// 1 + 2;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Expr(&'a Expr<'a, Ex, En>),
    /// Break inside a loop or switch statement.
    ///
    /// break;
    Break,
    /// Continue inside a loop or switch statement.
    ///
    /// continue;
    Continue,
    /// Throw an exception.
    ///
    /// throw $foo;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Throw(&'a Expr<'a, Ex, En>),
    /// Return, with an optional value.
    ///
    /// return;
    /// return $foo;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Return(Option<&'a Expr<'a, Ex, En>>),
    /// Yield break, terminating the current generator. This behaves like
    /// return; but is more explicit, and ensures the function is treated
    /// as a generator.
    ///
    /// yield break;
    YieldBreak,
    /// Concurrent block. All the await expressions are awaited at the
    /// same time, similar to genva().
    ///
    /// We store the desugared form. In the below example, the list is:
    /// [('__tmp$1', f()), (__tmp$2, g()), (None, h())]
    /// and the block assigns the temporary variables back to the locals.
    /// { $foo = __tmp$1; $bar = __tmp$2; }
    ///
    /// concurrent {
    /// $foo = await f();
    /// $bar = await g();
    /// await h();
    /// }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Awaitall(
        &'a (
            &'a [(Option<&'a Lid<'a>>, &'a Expr<'a, Ex, En>)],
            &'a Block<'a, Ex, En>,
        ),
    ),
    /// If statement.
    ///
    /// if ($foo) { ... } else { ... }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    If(
        &'a (
            &'a Expr<'a, Ex, En>,
            &'a Block<'a, Ex, En>,
            &'a Block<'a, Ex, En>,
        ),
    ),
    /// Do-while loop.
    ///
    /// do {
    /// bar();
    /// } while($foo)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Do(&'a (&'a Block<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
    /// While loop.
    ///
    /// while ($foo) {
    /// bar();
    /// }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    While(&'a (&'a Expr<'a, Ex, En>, &'a Block<'a, Ex, En>)),
    /// Initialize a value that is automatically disposed of.
    ///
    /// using $foo = bar(); // disposed at the end of the function
    /// using ($foo = bar(), $baz = quux()) {} // disposed after the block
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Using(&'a UsingStmt<'a, Ex, En>),
    /// For loop. The initializer and increment parts can include
    /// multiple comma-separated statements. The termination condition is
    /// optional.
    ///
    /// for ($i = 0; $i < 100; $i++) { ... }
    /// for ($x = 0, $y = 0; ; $x++, $y++) { ... }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
    /// switch ($foo) {
    /// case X:
    /// bar();
    /// break;
    /// default:
    /// baz();
    /// break;
    /// }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Switch(
        &'a (
            &'a Expr<'a, Ex, En>,
            &'a [Case<'a, Ex, En>],
            Option<DefaultCase<'a, Ex, En>>,
        ),
    ),
    /// For-each loop.
    ///
    /// foreach ($items as $item) { ... }
    /// foreach ($items as $key => value) { ... }
    /// foreach ($items await as $item) { ... } // AsyncIterator<_>
    /// foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Foreach(
        &'a (
            &'a Expr<'a, Ex, En>,
            AsExpr<'a, Ex, En>,
            &'a Block<'a, Ex, En>,
        ),
    ),
    /// Try statement, with catch blocks and a finally block.
    ///
    /// try {
    /// foo();
    /// } catch (SomeException $e) {
    /// bar();
    /// } finally {
    /// baz();
    /// }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Try(
        &'a (
            &'a Block<'a, Ex, En>,
            &'a [&'a Catch<'a, Ex, En>],
            &'a Block<'a, Ex, En>,
        ),
    ),
    /// No-op, the empty statement.
    ///
    /// {}
    /// while (true) ;
    /// if ($foo) {} // the else is Noop here
    Noop,
    /// Block, a list of statements in curly braces.
    ///
    /// { $foo = 42; }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Block(&'a Block<'a, Ex, En>),
    /// The mode tag at the beginning of a file.
    /// TODO: this really belongs in def.
    ///
    /// <?hh
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Markup(&'a Pstring<'a>),
    /// Used in IFC to track type inference environments. Not user
    /// denotable.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AssertEnv(
        &'a (
            oxidized::aast::EnvAnnot,
            &'a LocalIdMap<'a, (&'a Pos<'a>, Ex)>,
        ),
    ),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Stmt_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Stmt_<'arena, Ex, En>);

pub use oxidized::aast::EnvAnnot;

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
#[repr(C, u8)]
pub enum AsExpr<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AsV(&'a Expr<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AsKv(&'a (&'a Expr<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AwaitAsV(&'a (&'a Pos<'a>, &'a Expr<'a, Ex, En>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AwaitAsKv(&'a (&'a Pos<'a>, &'a Expr<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for AsExpr<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(AsExpr<'arena, Ex, En>);

pub type Block<'a, Ex, En> = [&'a Stmt<'a, Ex, En>];

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
#[repr(C, u8)]
pub enum ClassId_<'a, Ex, En> {
    /// The class ID of the parent of the lexically scoped class.
    ///
    /// In a trait, it is the parent class ID of the using class.
    ///
    /// parent::some_meth()
    /// parent::$prop = 1;
    /// new parent();
    CIparent,
    /// The class ID of the lexically scoped class.
    ///
    /// In a trait, it is the class ID of the using class.
    ///
    /// self::some_meth()
    /// self::$prop = 1;
    /// new self();
    CIself,
    /// The class ID of the late static bound class.
    ///
    /// https://www.php.net/manual/en/language.oop5.late-static-bindings.php
    ///
    /// In a trait, it is the late static bound class ID of the using class.
    ///
    /// static::some_meth()
    /// static::$prop = 1;
    /// new static();
    CIstatic,
    /// Dynamic class name.
    ///
    /// TODO: Syntactically this can only be an Lvar/This/Lplaceholder.
    /// We should use lid rather than expr.
    ///
    /// // Assume $d has type dynamic.
    /// $d::some_meth();
    /// $d::$prop = 1;
    /// new $d();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CIexpr(&'a Expr<'a, Ex, En>),
    /// Explicit class name. This is the common case.
    ///
    /// Foop::some_meth()
    /// Foo::$prop = 1;
    /// new Foo();
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
#[repr(C, u8)]
pub enum CollectionTarg<'a, Ex> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CollectionTV(&'a Targ<'a, Ex>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
#[repr(C, u8)]
pub enum FunctionPtrId<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FPId(&'a Sid<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FPClassConst(&'a (&'a ClassId<'a, Ex, En>, &'a Pstring<'a>)),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for FunctionPtrId<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(FunctionPtrId<'arena, Ex, En>);

/// An expression tree literal consists of a hint, splices, and
/// expressions. Consider this example:
///
/// Foo`1 + ${$x} + ${bar()}`
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
#[rust_to_ocaml(prefix = "et_")]
#[repr(C)]
pub struct ExpressionTree<'a, Ex, En> {
    /// The hint before the backtick, so Foo in this example.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub hint: &'a Hint<'a>,
    /// The values spliced into expression tree at runtime are assigned
    /// to temporaries.
    ///
    /// $0tmp1 = $x; $0tmp2 = bar();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub splices: &'a [&'a Stmt<'a, Ex, En>],
    /// The list of global functions and static methods assigned to
    /// temporaries.
    ///
    /// $0fp1 = foo<>;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub function_pointers: &'a [&'a Stmt<'a, Ex, En>],
    /// The expression that gets type checked.
    ///
    /// 1 + $0tmp1 + $0tmp2
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub virtualized_expr: &'a Expr<'a, Ex, En>,
    /// The expression that's executed at runtime.
    ///
    /// Foo::makeTree($v ==> $v->visitBinOp(...))
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub runtime_expr: &'a Expr<'a, Ex, En>,
    /// Position of the first $$ in a splice that refers
    /// to a variable outside the Expression Tree
    ///
    /// $x |> Code`${ $$ }` // Pos of the $$
    /// Code`${ $x |> foo($$) }` // None
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub dollardollar_pos: Option<&'a Pos<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ExpressionTree<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ExpressionTree<'arena, Ex, En>);

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
#[repr(C, u8)]
pub enum Expr_<'a, Ex, En> {
    /// darray literal.
    ///
    /// darray['x' => 0, 'y' => 1]
    /// darray<string, int>['x' => 0, 'y' => 1]
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Darray(
        &'a (
            Option<&'a (&'a Targ<'a, Ex>, &'a Targ<'a, Ex>)>,
            &'a [(&'a Expr<'a, Ex, En>, &'a Expr<'a, Ex, En>)],
        ),
    ),
    /// varray literal.
    ///
    /// varray['hello', 'world']
    /// varray<string>['hello', 'world']
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Varray(&'a (Option<&'a Targ<'a, Ex>>, &'a [&'a Expr<'a, Ex, En>])),
    /// Shape literal.
    ///
    /// shape('x' => 1, 'y' => 2)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Shape(&'a [(ast_defs::ShapeFieldName<'a>, &'a Expr<'a, Ex, En>)]),
    /// Collection literal for indexable structures.
    ///
    /// Vector {1, 2}
    /// ImmVector {}
    /// Set<string> {'foo', 'bar'}
    /// vec[1, 2]
    /// keyset[]
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ValCollection(
        &'a (
            oxidized::aast::VcKind,
            Option<&'a Targ<'a, Ex>>,
            &'a [&'a Expr<'a, Ex, En>],
        ),
    ),
    /// Collection literal for key-value structures.
    ///
    /// dict['x' => 1, 'y' => 2]
    /// Map<int, string> {}
    /// ImmMap {}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    KeyValCollection(
        &'a (
            oxidized::aast::KvcKind,
            Option<&'a (&'a Targ<'a, Ex>, &'a Targ<'a, Ex>)>,
            &'a [&'a Field<'a, Ex, En>],
        ),
    ),
    /// Null literal.
    ///
    /// null
    Null,
    /// The local variable representing the current class instance.
    ///
    /// $this
    This,
    /// Boolean literal.
    ///
    /// true
    True,
    /// Boolean literal.
    ///
    /// false
    False,
    /// The empty expression.
    ///
    /// list(, $y) = vec[1, 2] // Omitted is the first expression inside list()
    Omitted,
    /// An identifier. Used for method names and global constants.
    ///
    /// SOME_CONST
    /// $x->foo() // id: "foo"
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Id(&'a Sid<'a>),
    /// Local variable.
    ///
    /// $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Lvar(&'a Lid<'a>),
    /// The extra variable in a pipe expression.
    ///
    /// $$
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Dollardollar(&'a Lid<'a>),
    /// Clone expression.
    ///
    /// clone $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Clone(&'a Expr<'a, Ex, En>),
    /// Array indexing.
    ///
    /// $foo[]
    /// $foo[$bar]
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ArrayGet(&'a (&'a Expr<'a, Ex, En>, Option<&'a Expr<'a, Ex, En>>)),
    /// Instance property or method access.
    /// prop_or_method is
    ///   Is_prop for property access
    ///   Is_method for method call, only possible when the node is
    ///   the receiver in a Call node.
    ///
    ///   $foo->bar      // OG_nullthrows, Is_prop: access named property
    ///   $foo->bar()    // OG_nullthrows, Is_method: call named method
    ///   ($foo->bar)()  // OG_nullthrows, Is_prop: call lambda stored in named property
    ///   $foo?->bar     // OG_nullsafe,   Is_prop
    ///   $foo?->bar()   // OG_nullsafe,   Is_method
    ///   ($foo?->bar)() // OG_nullsafe,   Is_prop
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ObjGet(
        &'a (
            &'a Expr<'a, Ex, En>,
            &'a Expr<'a, Ex, En>,
            oxidized::aast::OgNullFlavor,
            oxidized::aast::PropOrMethod,
        ),
    ),
    /// Static property or method access.
    ///
    /// Foo::$bar               // Is_prop
    /// $some_classname::$bar   // Is_prop
    /// Foo::${$bar}            // Is_prop, only in partial mode
    ///
    /// Foo::bar();             // Is_method
    /// Foo::$bar();            // Is_method, name stored in local $bar
    /// (Foo::$bar)();          // Is_prop: call lambda stored in property Foo::$bar
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ClassGet(
        &'a (
            &'a ClassId<'a, Ex, En>,
            ClassGetExpr<'a, Ex, En>,
            oxidized::aast::PropOrMethod,
        ),
    ),
    /// Class constant or static method call. As a standalone expression,
    /// this is a class constant. Inside a Call node, this is a static
    /// method call.
    ///
    /// This is not ambiguous, because constants are not allowed to
    /// contain functions.
    ///
    /// Foo::some_const // Class_const
    /// Foo::someStaticMeth() // Call (Class_const)
    ///
    /// This syntax is used for both static and instance methods when
    /// calling the implementation on the superclass.
    ///
    /// parent::someStaticMeth()
    /// parent::someInstanceMeth()
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ClassConst(&'a (&'a ClassId<'a, Ex, En>, &'a Pstring<'a>)),
    /// Function or method call.
    ///
    /// foo()
    /// $x()
    /// foo<int>(1, 2, ...$rest)
    /// $x->foo()
    /// bar(inout $x);
    /// foobar(inout $x[0])
    ///
    /// async { return 1; }
    /// // lowered to:
    /// (async () ==> { return 1; })()
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Call(
        &'a (
            &'a Expr<'a, Ex, En>,
            &'a [&'a Targ<'a, Ex>],
            &'a [(ast_defs::ParamKind<'a>, &'a Expr<'a, Ex, En>)],
            Option<&'a Expr<'a, Ex, En>>,
        ),
    ),
    /// A reference to a function or method.
    ///
    /// foo_fun<>
    /// FooCls::meth<int>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FunctionPointer(&'a (FunctionPtrId<'a, Ex, En>, &'a [&'a Targ<'a, Ex>])),
    /// Integer literal.
    ///
    /// 42
    /// 0123 // octal
    /// 0xBEEF // hexadecimal
    /// 0b11111111 // binary
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Int(&'a str),
    /// Float literal.
    ///
    /// 1.0
    /// 1.2e3
    /// 7E-10
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Float(&'a str),
    /// String literal.
    ///
    /// "foo"
    /// 'foo'
    ///
    /// <<<DOC
    /// foo
    /// DOC
    ///
    /// <<<'DOC'
    /// foo
    /// DOC
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    String(&'a bstr::BStr),
    /// Interpolated string literal.
    ///
    /// "hello $foo $bar"
    ///
    /// <<<DOC
    /// hello $foo $bar
    /// DOC
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    String2(&'a [&'a Expr<'a, Ex, En>]),
    /// Prefixed string literal. Only used for regular expressions.
    ///
    /// re"foo"
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    PrefixedString(&'a (&'a str, &'a Expr<'a, Ex, En>)),
    /// Yield expression. The enclosing function should have an Iterator
    /// return type.
    ///
    /// yield $foo // enclosing function returns an Iterator
    /// yield $foo => $bar // enclosing function returns a KeyedIterator
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Yield(&'a Afield<'a, Ex, En>),
    /// Await expression.
    ///
    /// await $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Await(&'a Expr<'a, Ex, En>),
    /// Readonly expression.
    ///
    /// readonly $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ReadonlyExpr(&'a Expr<'a, Ex, En>),
    /// Tuple expression.
    ///
    /// tuple("a", 1, $foo)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tuple(&'a [&'a Expr<'a, Ex, En>]),
    /// List expression, only used in destructuring. Allows any arbitrary
    /// lvalue as a subexpression. May also nest.
    ///
    /// list($x, $y) = vec[1, 2];
    /// list(, $y) = vec[1, 2]; // skipping items
    /// list(list($x)) = vec[vec[1]]; // nesting
    /// list($v[0], $x[], $y->foo) = $blah;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    List(&'a [&'a Expr<'a, Ex, En>]),
    /// Cast expression, converting a value to a different type. Only
    /// primitive types are supported in the hint position.
    ///
    /// (int)$foo
    /// (string)$foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Cast(&'a (&'a Hint<'a>, &'a Expr<'a, Ex, En>)),
    /// Unary operator.
    ///
    /// !$foo
    /// -$foo
    /// +$foo
    /// $foo++
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Unop(&'a (oxidized::ast_defs::Uop, &'a Expr<'a, Ex, En>)),
    /// Binary operator.
    ///
    /// $foo + $bar
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Binop(
        &'a (
            ast_defs::Bop<'a>,
            &'a Expr<'a, Ex, En>,
            &'a Expr<'a, Ex, En>,
        ),
    ),
    /// Pipe expression. The lid is the ID of the $$ that is implicitly
    /// declared by this pipe.
    ///
    /// See also Dollardollar.
    ///
    /// foo() |> bar(1, $$) // equivalent: bar(1, foo())
    ///
    /// $$ is not required on the RHS of pipe expressions, but it's
    /// pretty pointless to use pipes without $$.
    ///
    /// foo() |> bar(); // equivalent: foo(); bar();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Pipe(&'a (&'a Lid<'a>, &'a Expr<'a, Ex, En>, &'a Expr<'a, Ex, En>)),
    /// Ternary operator, or elvis operator.
    ///
    /// $foo ? $bar : $baz // ternary
    /// $foo ?: $baz // elvis
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Eif(
        &'a (
            &'a Expr<'a, Ex, En>,
            Option<&'a Expr<'a, Ex, En>>,
            &'a Expr<'a, Ex, En>,
        ),
    ),
    /// Is operator.
    ///
    /// $foo is SomeType
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Is(&'a (&'a Expr<'a, Ex, En>, &'a Hint<'a>)),
    /// As operator.
    ///
    /// $foo as int
    /// $foo ?as int
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    As(&'a (&'a Expr<'a, Ex, En>, &'a Hint<'a>, bool)),
    /// Upcast operator.
    ///
    /// $foo : int
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Upcast(&'a (&'a Expr<'a, Ex, En>, &'a Hint<'a>)),
    /// Instantiation.
    ///
    /// new Foo(1, 2);
    /// new Foo<int, T>();
    /// new Foo('blah', ...$rest);
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
    /// function($x) { return $x; }
    /// function(int $x): int { return $x; }
    /// function($x) use ($y) { return $y; }
    /// function($x): int use ($y, $z) { return $x + $y + $z; }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Efun(&'a (&'a Fun_<'a, Ex, En>, &'a [&'a Lid<'a>])),
    /// Hack lambda. Captures variables automatically.
    ///
    /// $x ==> $x
    /// (int $x): int ==> $x + $other
    /// ($x, $y) ==> { return $x + $y; }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Lfun(&'a (&'a Fun_<'a, Ex, En>, &'a [&'a Lid<'a>])),
    /// XHP expression. May contain interpolated expressions.
    ///
    /// <foo x="hello" y={$foo}>hello {$bar}</foo>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Xml(
        &'a (
            &'a ClassName<'a>,
            &'a [XhpAttribute<'a, Ex, En>],
            &'a [&'a Expr<'a, Ex, En>],
        ),
    ),
    /// Include or require expression.
    ///
    /// require('foo.php')
    /// require_once('foo.php')
    /// include('foo.php')
    /// include_once('foo.php')
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Import(&'a (oxidized::aast::ImportFlavor, &'a Expr<'a, Ex, En>)),
    /// Collection literal.
    ///
    /// TODO: T38184446 this is redundant with ValCollection/KeyValCollection.
    ///
    /// Vector {}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
    /// Foo`1 + bar()`
    /// Foo`(() ==> { while(true) {} })()` // not an infinite loop at runtime
    ///
    /// Splices are evaluated as normal Hack code. The following two expression trees
    /// are equivalent. See also `ET_Splice`.
    ///
    /// Foo`1 + ${do_stuff()}`
    ///
    /// $x = do_stuff();
    /// Foo`1 + ${$x}`
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ExpressionTree(&'a ExpressionTree<'a, Ex, En>),
    /// Placeholder local variable.
    ///
    /// $_
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Lplaceholder(&'a Pos<'a>),
    /// Global function reference.
    ///
    /// fun('foo')
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FunId(&'a Sid<'a>),
    /// Instance method reference on a specific instance.
    ///
    /// TODO: This is only created in naming, and ought to happen in
    /// lowering or be removed. The emitter just sees a normal Call.
    ///
    /// inst_meth($f, 'some_meth') // equivalent: $f->some_meth<>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    MethodId(&'a (&'a Expr<'a, Ex, En>, &'a Pstring<'a>)),
    /// Instance method reference that can be called with an instance.
    ///
    /// meth_caller(FooClass::class, 'some_meth')
    /// meth_caller('FooClass', 'some_meth')
    ///
    /// These examples are equivalent to:
    ///
    /// (FooClass $f, ...$args) ==> $f->some_meth(...$args)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    MethodCaller(&'a (&'a ClassName<'a>, &'a Pstring<'a>)),
    /// Static method reference.
    ///
    /// class_meth('FooClass', 'some_static_meth')
    /// // equivalent: FooClass::some_static_meth<>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    SmethodId(&'a (&'a ClassId<'a, Ex, En>, &'a Pstring<'a>)),
    /// Pair literal.
    ///
    /// Pair {$foo, $bar}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
    /// ${$foo}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ETSplice(&'a Expr<'a, Ex, En>),
    /// Label used for enum classes.
    ///
    /// enum_name#label_name or #label_name
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
    ///          function f(int $x): void { /* ... */ }
    ///
    ///          function g(float $x): void {
    ///             f(unsafe_cast<float,int>($x));
    ///          }
    /// ```
    /// After typing, this is represented by the following TAST fragment
    /// ```
    ///          Call
    ///            ( ( (..., function(int $x): void), Id (..., "\f"))
    ///            , []
    ///            , [ ( (..., int)
    ///                , Hole
    ///                    ( ((..., float), Lvar (..., $x))
    ///                    , float
    ///                    , int
    ///                    , UnsafeCast
    ///                    )
    ///                )
    ///              ]
    ///            , None
    ///            )
    /// ```
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hole(&'a (&'a Expr<'a, Ex, En>, Ex, Ex, HoleSource<'a>)),
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
#[repr(C, u8)]
pub enum HoleSource<'a> {
    Typing,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    UnsafeCast(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    EnforcedCast(&'a [&'a Hint<'a>]),
}
impl<'a> TrivialDrop for HoleSource<'a> {}
arena_deserializer::impl_deserialize_in_arena!(HoleSource<'arena>);

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
#[repr(C, u8)]
pub enum Afield<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AFvalue(&'a Expr<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
#[rust_to_ocaml(prefix = "xs_")]
#[repr(C)]
pub struct XhpSimple<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
#[repr(C, u8)]
pub enum XhpAttribute<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    XhpSimple(&'a XhpSimple<'a, Ex, En>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    XhpSpread(&'a Expr<'a, Ex, En>),
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for XhpAttribute<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(XhpAttribute<'arena, Ex, En>);

pub use oxidized::aast::IsVariadic;

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
#[rust_to_ocaml(prefix = "param_")]
#[repr(C)]
pub struct FunParam<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_hint: &'a TypeHint<'a, Ex>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub is_variadic: &'a oxidized::aast::IsVariadic,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: Option<&'a Expr<'a, Ex, En>>,
    pub readonly: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub callconv: ast_defs::ParamKind<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    pub visibility: Option<oxidized::aast::Visibility>,
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
#[rust_to_ocaml(prefix = "f_")]
#[repr(C)]
pub struct Fun_<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    pub readonly_this: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    pub readonly_ret: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ret: &'a TypeHint<'a, Ex>,
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
    pub fun_kind: oxidized::ast_defs::FunKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Fun_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Fun_<'arena, Ex, En>);

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
#[repr(C)]
pub struct Targ<'a, Ex>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
);
impl<'a, Ex: TrivialDrop> TrivialDrop for Targ<'a, Ex> {}
arena_deserializer::impl_deserialize_in_arena!(Targ<'arena, Ex>);

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
#[rust_to_ocaml(prefix = "fa_")]
#[repr(C)]
pub struct FileAttribute<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
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
#[rust_to_ocaml(prefix = "tp_")]
#[repr(C)]
pub struct Tparam<'a, Ex, En> {
    pub variance: oxidized::ast_defs::Variance,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub parameters: &'a [&'a Tparam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraints: &'a [(oxidized::ast_defs::ConstraintKind, &'a Hint<'a>)],
    pub reified: oxidized::aast::ReifyKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Tparam<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Tparam<'arena, Ex, En>);

pub use oxidized::aast::EmitId;
pub use oxidized::aast::RequireKind;

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
#[rust_to_ocaml(prefix = "c_")]
#[repr(C)]
pub struct Class_<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassishKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a ClassName<'a>,
    /// The type parameters of a class A<T> (T is the parameter)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub extends: &'a [&'a ClassHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub uses: &'a [&'a TraitHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attr_uses: &'a [&'a XhpAttrHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_category: Option<&'a (&'a Pos<'a>, &'a [&'a Pstring<'a>])>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub reqs: &'a [(&'a ClassHint<'a>, &'a oxidized::aast::RequireKind)],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub implements: &'a [&'a ClassHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub consts: &'a [&'a ClassConst<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub typeconsts: &'a [&'a ClassTypeconstDef<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub vars: &'a [&'a ClassVar<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub methods: &'a [&'a Method_<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_children: &'a [(&'a Pos<'a>, &'a XhpChild<'a>)],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attrs: &'a [&'a XhpAttr<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub docs_url: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub enum_: Option<&'a Enum_<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub emit_id: Option<oxidized::aast::EmitId>,
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<Sid<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Class_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Class_<'arena, Ex, En>);

pub type ClassHint<'a> = Hint<'a>;

pub type TraitHint<'a> = Hint<'a>;

pub type XhpAttrHint<'a> = Hint<'a>;

pub use oxidized::aast::XhpAttrTag;

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
#[repr(C)]
pub struct XhpAttr<'a, Ex, En>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a TypeHint<'a, Ex>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a ClassVar<'a, Ex, En>,
    pub Option<oxidized::aast::XhpAttrTag>,
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
#[repr(C, u8)]
pub enum ClassConstKind<'a, Ex, En> {
    /// CCAbstract represents the states
    ///    abstract const int X;
    ///    abstract const int Y = 4;
    /// The expr option is a default value
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CCAbstract(Option<&'a Expr<'a, Ex, En>>),
    /// CCConcrete represents
    ///    const int Z = 4;
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
#[rust_to_ocaml(prefix = "cc_")]
#[repr(C)]
pub struct ClassConst<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: ClassConstKind<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for ClassConst<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(ClassConst<'arena, Ex, En>);

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
#[rust_to_ocaml(prefix = "c_atc_")]
#[repr(C)]
pub struct ClassAbstractTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub as_constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub super_constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
#[rust_to_ocaml(prefix = "c_tconst_")]
#[repr(C)]
pub struct ClassTypeconstDef<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: ClassTypeconst<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
#[rust_to_ocaml(prefix = "xai_")]
#[repr(C)]
pub struct XhpAttrInfo<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub like: Option<&'a Pos<'a>>,
    pub tag: Option<oxidized::aast::XhpAttrTag>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
#[rust_to_ocaml(prefix = "cv_")]
#[repr(C)]
pub struct ClassVar<'a, Ex, En> {
    pub final_: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attr: Option<&'a XhpAttrInfo<'a>>,
    pub abstract_: bool,
    pub readonly: bool,
    pub visibility: oxidized::aast::Visibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a TypeHint<'a, Ex>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: Option<&'a Expr<'a, Ex, En>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
#[rust_to_ocaml(prefix = "m_")]
#[repr(C)]
pub struct Method_<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub readonly_this: bool,
    pub visibility: oxidized::aast::Visibility,
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
    pub fun_kind: oxidized::ast_defs::FunKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    pub readonly_ret: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ret: &'a TypeHint<'a, Ex>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, En: TrivialDrop> TrivialDrop for Method_<'a, Ex, En> {}
arena_deserializer::impl_deserialize_in_arena!(Method_<'arena, Ex, En>);

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
    pub constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, En>],
    pub mode: oxidized::file_info::Mode,
    pub vis: oxidized::aast::TypedefVisibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    pub emit_id: Option<oxidized::aast::EmitId>,
    pub is_ctx: bool,
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<Sid<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub docs_url: Option<&'a str>,
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
#[rust_to_ocaml(prefix = "cst_")]
#[repr(C)]
pub struct Gconst<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub value: &'a Expr<'a, Ex, En>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    pub emit_id: Option<oxidized::aast::EmitId>,
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
#[rust_to_ocaml(prefix = "fd_")]
#[repr(C)]
pub struct FunDef<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, En>],
    pub mode: oxidized::file_info::Mode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fun: &'a Fun_<'a, Ex, En>,
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<Sid<'a>>,
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
#[rust_to_ocaml(prefix = "md_")]
#[repr(C)]
pub struct ModuleDef<'a, Ex, En> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: ast_defs::Id<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, En>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    pub mode: oxidized::file_info::Mode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
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
#[serde(bound(
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
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
    Namespace(&'a (Sid<'a>, &'a [Def<'a, Ex, En>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    NamespaceUse(&'a [(oxidized::aast::NsKind, Sid<'a>, Sid<'a>)]),
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

pub use oxidized::aast::NsKind;
