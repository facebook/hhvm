// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<2fc8f90532ee63701250e31c354e4176>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use aast_defs::*;
pub use doc_comment::DocComment;

/// Aast.program represents the top-level definitions in a Hack program.
/// ex: Expression annotation type (when typechecking, the inferred type)
/// fb: Function body tag (e.g. has naming occurred)
/// en: Environment (tracking state inside functions and classes)
/// hi: Hint annotation (when typechecking it will be the localized type hint or the
/// inferred missing type if the hint is missing)
pub type Program<'a, Ex, Fb, En, Hi> = [Def<'a, Ex, Fb, En, Hi>];

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Stmt<'a, Ex, Fb, En, Hi>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Stmt_<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Stmt<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Stmt<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum Stmt_<'a, Ex, Fb, En, Hi> {
    /// Marker for a switch statement that falls through.
    ///
    /// // FALLTHROUGH
    Fallthrough,
    /// Standalone expression.
    ///
    /// 1 + 2;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Expr(&'a Expr<'a, Ex, Fb, En, Hi>),
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
    Throw(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Return, with an optional value.
    ///
    /// return;
    /// return $foo;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Return(Option<&'a Expr<'a, Ex, Fb, En, Hi>>),
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
            &'a [(Option<&'a Lid<'a>>, &'a Expr<'a, Ex, Fb, En, Hi>)],
            &'a Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// If statement.
    ///
    /// if ($foo) { ... } else { ... }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    If(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Block<'a, Ex, Fb, En, Hi>,
            &'a Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// Do-while loop.
    ///
    /// do {
    /// bar();
    /// } while($foo)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Do(&'a (&'a Block<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// While loop.
    ///
    /// while ($foo) {
    /// bar();
    /// }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    While(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Block<'a, Ex, Fb, En, Hi>)),
    /// Initialize a value that is automatically disposed of.
    ///
    /// using $foo = bar(); // disposed at the end of the function
    /// using ($foo = bar(), $baz = quux()) {} // disposed after the block
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Using(&'a UsingStmt<'a, Ex, Fb, En, Hi>),
    /// For loop. The initializer and increment parts can include
    /// multiple comma-separated statements. The termination condition is
    /// optional.
    ///
    /// for ($i = 0; $i < 100; $i++) { ... }
    /// for ($x = 0, $y = 0; ; $x++, $y++) { ... }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    For(
        &'a (
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
            &'a Block<'a, Ex, Fb, En, Hi>,
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
    Switch(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a [Case<'a, Ex, Fb, En, Hi>])),
    /// For-each loop.
    ///
    /// foreach ($items as $item) { ... }
    /// foreach ($items as $key => value) { ... }
    /// foreach ($items await as $item) { ... } // AsyncIterator<_>
    /// foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Foreach(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            AsExpr<'a, Ex, Fb, En, Hi>,
            &'a Block<'a, Ex, Fb, En, Hi>,
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
            &'a Block<'a, Ex, Fb, En, Hi>,
            &'a [&'a Catch<'a, Ex, Fb, En, Hi>],
            &'a Block<'a, Ex, Fb, En, Hi>,
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
    Block(&'a Block<'a, Ex, Fb, En, Hi>),
    /// The mode tag at the beginning of a file.
    /// TODO: this really belongs in def.
    ///
    /// <?hh
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Markup(&'a Pstring<'a>),
    /// Used in IFC to track type inference environments. Not user
    /// denotable.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AssertEnv(&'a (oxidized::aast::EnvAnnot, &'a LocalIdMap<'a, Ex>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Stmt_<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Stmt_<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct UsingStmt<'a, Ex, Fb, En, Hi> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub exprs: (&'a Pos<'a>, &'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub block: &'a Block<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for UsingStmt<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(UsingStmt<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum AsExpr<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AsV(&'a Expr<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AsKv(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AwaitAsV(&'a (&'a Pos<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AwaitAsKv(
        &'a (
            &'a Pos<'a>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for AsExpr<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(AsExpr<'arena, Ex, Fb, En, Hi>);

pub type Block<'a, Ex, Fb, En, Hi> = [&'a Stmt<'a, Ex, Fb, En, Hi>];

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct ClassId<'a, Ex, Fb, En, Hi>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  ClassId_<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassId<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(ClassId<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum ClassId_<'a, Ex, Fb, En, Hi> {
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
    /// TODO: Syntactically this can only be an Lvar/This/Lplacehodller.
    /// We should use lid rather than expr.
    ///
    /// // Assume $d has type dynamic.
    /// $d::some_meth();
    /// $d::$prop = 1;
    /// new $d();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CIexpr(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Explicit class name. This is the common case.
    ///
    /// Foop::some_meth()
    /// Foo::$prop = 1;
    /// new Foo();
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CI(&'a Sid<'a>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassId_<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(ClassId_<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Expr<'a, Ex, Fb, En, Hi>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Expr_<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Expr<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Expr<'arena, Ex, Fb, En, Hi>);

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
#[serde(bound(deserialize = "Hi: 'de + arena_deserializer::DeserializeInArena<'de>"))]
pub enum CollectionTarg<'a, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CollectionTV(&'a Targ<'a, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CollectionTKV(&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)),
}
impl<'a, Hi: TrivialDrop> TrivialDrop for CollectionTarg<'a, Hi> {}
arena_deserializer::impl_deserialize_in_arena!(CollectionTarg<'arena, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum FunctionPtrId<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FPId(&'a Sid<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FPClassConst(&'a (&'a ClassId<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunctionPtrId<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(FunctionPtrId<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct ExpressionTree<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub hint: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub splices: &'a Block<'a, Ex, Fb, En, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub virtualized_expr: &'a Expr<'a, Ex, Fb, En, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub runtime_expr: &'a Expr<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ExpressionTree<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(ExpressionTree<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum Expr_<'a, Ex, Fb, En, Hi> {
    /// darray literal.
    ///
    /// darray['x' => 0, 'y' => 1]
    /// darray<string, int>['x' => 0, 'y' => 1]
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Darray(
        &'a (
            Option<&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)>,
            &'a [(&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)],
        ),
    ),
    /// varray literal.
    ///
    /// varray['hello', 'world']
    /// varray<string>['hello', 'world']
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Varray(&'a (Option<&'a Targ<'a, Hi>>, &'a [&'a Expr<'a, Ex, Fb, En, Hi>])),
    /// Shape literal.
    ///
    /// shape('x' => 1, 'y' => 2)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Shape(&'a [(ast_defs::ShapeFieldName<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)]),
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
            Option<&'a Targ<'a, Hi>>,
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
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
            Option<&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)>,
            &'a [&'a Field<'a, Ex, Fb, En, Hi>],
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
    Clone(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Array indexing.
    ///
    /// $foo[]
    /// $foo[$bar]
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ArrayGet(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
        ),
    ),
    /// Instance property or method access.  is_prop_call is always
    /// false, except when inside a call is accessing a property.
    ///
    /// $foo->bar // (Obj_get false) property access
    /// $foo->bar() // (Call (Obj_get false)) method call
    /// ($foo->bar)() // (Call (Obj_get true)) call lambda stored in property
    /// $foo?->bar // nullsafe access
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ObjGet(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            oxidized::aast::OgNullFlavor,
            bool,
        ),
    ),
    /// Static property access.
    ///
    /// Foo::$bar
    /// $some_classname::$bar
    /// Foo::${$bar} // only in partial mode
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ClassGet(
        &'a (
            &'a ClassId<'a, Ex, Fb, En, Hi>,
            ClassGetExpr<'a, Ex, Fb, En, Hi>,
            bool,
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
    ClassConst(&'a (&'a ClassId<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
    /// Function or method call.
    ///
    /// foo()
    /// $x()
    /// foo<int>(1, 2, ...$rest)
    /// $x->foo()
    ///
    /// async { return 1; }
    /// // lowered to:
    /// (async () ==> { return 1; })()
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Call(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a [&'a Targ<'a, Hi>],
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
        ),
    ),
    /// A reference to a function or method.
    ///
    /// foo_fun<>
    /// FooCls::meth<int>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FunctionPointer(&'a (FunctionPtrId<'a, Ex, Fb, En, Hi>, &'a [&'a Targ<'a, Hi>])),
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
    String2(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    /// Prefixed string literal. Only used for regular expressions.
    ///
    /// re"foo"
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    PrefixedString(&'a (&'a str, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Yield expression. The enclosing function should have an Iterator
    /// return type.
    ///
    /// yield $foo // enclosing function returns an Iterator
    /// yield $foo => $bar // enclosing function returns a KeyedIterator
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Yield(&'a Afield<'a, Ex, Fb, En, Hi>),
    /// Await expression.
    ///
    /// await $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Await(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Readonly expression.
    ///
    /// readonly $foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ReadonlyExpr(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Tuple expression.
    ///
    /// tuple("a", 1, $foo)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tuple(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    /// List expression, only used in destructuring. Allows any arbitrary
    /// lvalue as a subexpression. May also nest.
    ///
    /// list($x, $y) = vec[1, 2];
    /// list(, $y) = vec[1, 2]; // skipping items
    /// list(list($x)) = vec[vec[1]]; // nesting
    /// list($v[0], $x[], $y->foo) = $blah;
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    List(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    /// Cast expression, converting a value to a different type. Only
    /// primitive types are supported in the hint position.
    ///
    /// (int)$foo
    /// (string)$foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Cast(&'a (&'a Hint<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Unary operator.
    ///
    /// !$foo
    /// -$foo
    /// +$foo
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Unop(&'a (oxidized::ast_defs::Uop, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Binary operator.
    ///
    /// $foo + $bar
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Binop(
        &'a (
            ast_defs::Bop<'a>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// Pipe expression. The lid is the ID of the $$ that is implicitly
    /// declared by this pipe.
    ///
    /// See also Dollardollar.
    ///
    /// $foo |> bar() // equivalent: bar($foo)
    /// $foo |> bar(1, $$) // equivalent: bar(1, $foo)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Pipe(
        &'a (
            &'a Lid<'a>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// Ternary operator, or elvis operator.
    ///
    /// $foo ? $bar : $baz // ternary
    /// $foo ?: $baz // elvis
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Eif(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// Is operator.
    ///
    /// $foo is SomeType
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Is(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Hint<'a>)),
    /// As operator.
    ///
    /// $foo as int
    /// $foo ?as int
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    As(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Hint<'a>, bool)),
    /// Instantiation.
    ///
    /// new Foo(1, 2);
    /// new Foo<int, T>();
    /// new Foo('blah', ...$rest);
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    New(
        &'a (
            &'a ClassId<'a, Ex, Fb, En, Hi>,
            &'a [&'a Targ<'a, Hi>],
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
            Ex,
        ),
    ),
    /// Record literal.
    ///
    /// MyRecord['x' => $foo, 'y' => $bar]
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Record(
        &'a (
            Sid<'a>,
            &'a [(&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)],
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
    Efun(&'a (&'a Fun_<'a, Ex, Fb, En, Hi>, &'a [&'a Lid<'a>])),
    /// Hack lambda. Captures variables automatically.
    ///
    /// $x ==> $x
    /// (int $x): int ==> $x + $other
    /// ($x, $y) ==> { return $x + $y; }
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Lfun(&'a (&'a Fun_<'a, Ex, Fb, En, Hi>, &'a [&'a Lid<'a>])),
    /// XHP expression. May contain interpolated expressions.
    ///
    /// <foo x="hello" y={$foo}>hello {$bar}</foo>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Xml(
        &'a (
            Sid<'a>,
            &'a [XhpAttribute<'a, Ex, Fb, En, Hi>],
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
        ),
    ),
    /// Explicit calling convention, used for inout. Inout supports any lvalue.
    ///
    /// TODO: This could be a flag on parameters in Call.
    ///
    /// foo(inout $x[0])
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Callconv(&'a (oxidized::ast_defs::ParamKind, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Include or require expression.
    ///
    /// require('foo.php')
    /// require_once('foo.php')
    /// include('foo.php')
    /// include_once('foo.php')
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Import(&'a (oxidized::aast::ImportFlavor, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Collection literal.
    ///
    /// TODO: T38184446 this is redundant with ValCollection/KeyValCollection.
    ///
    /// Vector {}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Collection(
        &'a (
            Sid<'a>,
            Option<CollectionTarg<'a, Hi>>,
            &'a [Afield<'a, Ex, Fb, En, Hi>],
        ),
    ),
    /// Expression tree literal. Expression trees are not evaluated at
    /// runtime, but desugared to an expression representing the code.
    ///
    /// Foo`1 + bar()`
    /// Foo`$x ==> $x * ${$value}` // splicing $value
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ExpressionTree(&'a ExpressionTree<'a, Ex, Fb, En, Hi>),
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
    MethodId(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
    /// Instance method reference that can be called with an instance.
    ///
    /// meth_caller(FooClass::class, 'some_meth')
    /// meth_caller('FooClass', 'some_meth')
    ///
    /// These examples are equivalent to:
    ///
    /// (FooClass $f, ...$args) ==> $f->some_meth(...$args)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    MethodCaller(&'a (Sid<'a>, &'a Pstring<'a>)),
    /// Static method reference.
    ///
    /// class_meth('FooClass', 'some_static_meth')
    /// // equivalent: FooClass::some_static_meth<>
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    SmethodId(&'a (&'a ClassId<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
    /// Pair literal.
    ///
    /// Pair {$foo, $bar}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Pair(
        &'a (
            Option<&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// Expression tree splice expression. Only valid inside an
    /// expression tree literal (backticks).
    ///
    /// ${$foo}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ETSplice(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Label used for enum classes.
    ///
    /// enum_name#label_name or #label_name
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    EnumClassLabel(&'a (Option<Sid<'a>>, &'a str)),
    /// Placeholder for expressions that aren't understood by parts of
    /// the toolchain.
    ///
    /// TODO: Remove.
    Any,
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
    Hole(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            Hi,
            Hi,
            &'a oxidized::aast::HoleSource,
        ),
    ),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Expr_<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Expr_<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum ClassGetExpr<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CGstring(&'a Pstring<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CGexpr(&'a Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassGetExpr<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(ClassGetExpr<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum Case<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Default(&'a (&'a Pos<'a>, &'a Block<'a, Ex, Fb, En, Hi>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Case(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Block<'a, Ex, Fb, En, Hi>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Case<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Case<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Catch<'a, Ex, Fb, En, Hi>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Lid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a Block<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Catch<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Catch<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Field<'a, Ex, Fb, En, Hi>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a Expr<'a, Ex, Fb, En, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a Expr<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Field<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Field<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum Afield<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AFvalue(&'a Expr<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    AFkvalue(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Afield<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Afield<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct XhpSimple<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a Pstring<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub type_: Hi,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: &'a Expr<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for XhpSimple<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(XhpSimple<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum XhpAttribute<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    XhpSimple(&'a XhpSimple<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    XhpSpread(&'a Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for XhpAttribute<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(XhpAttribute<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct FunParam<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: Ex,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_hint: &'a TypeHint<'a, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub is_variadic: &'a oxidized::aast::IsVariadic,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub readonly: Option<oxidized::ast_defs::ReadonlyKind>,
    pub callconv: Option<oxidized::ast_defs::ParamKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub visibility: Option<oxidized::aast::Visibility>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunParam<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(FunParam<'arena, Ex, Fb, En, Hi>);

/// Does this function/method take a variable number of arguments?
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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum FunVariadicity<'a, Ex, Fb, En, Hi> {
    /// Named variadic argument.
    ///
    /// function foo(int ...$args): void {}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FVvariadicArg(&'a FunParam<'a, Ex, Fb, En, Hi>),
    /// Unnamed variaidic argument. Partial mode only.
    ///
    /// function foo(...): void {}
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FVellipsis(&'a Pos<'a>),
    /// Function is not variadic, takes an exact number of arguments.
    FVnonVariadic,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunVariadicity<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(FunVariadicity<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Fun_<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    pub readonly_this: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    pub readonly_ret: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ret: &'a TypeHint<'a, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub unsafe_ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub body: &'a FuncBody<'a, Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Fun_<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Fun_<'arena, Ex, Fb, En, Hi>);

/// Naming has two phases and the annotation helps to indicate the phase.
/// In the first pass, it will perform naming on everything except for function
/// and method bodies and collect information needed. Then, another round of
/// naming is performed where function bodies are named. Thus, naming will
/// have named and unnamed variants of the annotation.
/// See BodyNamingAnnotation in nast.ml and the comment in naming.ml
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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct FuncBody<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ast: &'a Block<'a, Ex, Fb, En, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: Fb,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FuncBody<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(FuncBody<'arena, Ex, Fb, En, Hi>);

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
#[serde(bound(deserialize = "Hi: 'de + arena_deserializer::DeserializeInArena<'de>"))]
pub struct TypeHint<'a, Hi>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Hi,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a TypeHint_<'a>,
);
impl<'a, Hi: TrivialDrop> TrivialDrop for TypeHint<'a, Hi> {}
arena_deserializer::impl_deserialize_in_arena!(TypeHint<'arena, Hi>);

/// Explicit type argument to function, constructor, or collection literal.
/// 'hi = unit in NAST
/// 'hi = Typing_defs.(locl ty) in TAST,
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
#[serde(bound(deserialize = "Hi: 'de + arena_deserializer::DeserializeInArena<'de>"))]
pub struct Targ<'a, Hi>(
    #[serde(deserialize_with = "arena_deserializer::arena")] pub Hi,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
);
impl<'a, Hi: TrivialDrop> TrivialDrop for Targ<'a, Hi> {}
arena_deserializer::impl_deserialize_in_arena!(Targ<'arena, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct UserAttribute<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    /// user attributes are restricted to scalar values
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub params: &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for UserAttribute<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(UserAttribute<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct FileAttribute<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FileAttribute<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(FileAttribute<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Tparam<'a, Ex, Fb, En, Hi> {
    pub variance: oxidized::ast_defs::Variance,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub parameters: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraints: &'a [(oxidized::ast_defs::ConstraintKind, &'a Hint<'a>)],
    pub reified: oxidized::aast::ReifyKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Tparam<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Tparam<'arena, Ex, Fb, En, Hi>);

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
pub struct UseAsAlias<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Option<Sid<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pstring<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Option<Sid<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a [oxidized::aast::UseAsVisibility],
);
impl<'a> TrivialDrop for UseAsAlias<'a> {}
arena_deserializer::impl_deserialize_in_arena!(UseAsAlias<'arena>);

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
pub struct InsteadofAlias<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pstring<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a [Sid<'a>],
);
impl<'a> TrivialDrop for InsteadofAlias<'a> {}
arena_deserializer::impl_deserialize_in_arena!(InsteadofAlias<'arena>);

pub use oxidized::aast::IsExtends;

pub use oxidized::aast::EmitId;

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Class_<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    /// The type parameters of a class A<T> (T is the parameter)
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub extends: &'a [&'a ClassHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub uses: &'a [&'a TraitHint<'a>],
    /// PHP feature not supported in hack but required
    /// because we have runtime support.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub use_as_alias: &'a [&'a UseAsAlias<'a>],
    /// PHP feature not supported in hack but required
    /// because we have runtime support.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub insteadof_alias: &'a [&'a InsteadofAlias<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attr_uses: &'a [&'a XhpAttrHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_category: Option<&'a (&'a Pos<'a>, &'a [&'a Pstring<'a>])>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub reqs: &'a [(&'a ClassHint<'a>, &'a oxidized::aast::IsExtends)],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub implements: &'a [&'a ClassHint<'a>],
    pub support_dynamic_type: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub consts: &'a [&'a ClassConst<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub typeconsts: &'a [&'a ClassTypeconstDef<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub vars: &'a [&'a ClassVar<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub methods: &'a [&'a Method_<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub attributes: &'a [ClassAttr<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_children: &'a [(&'a Pos<'a>, &'a XhpChild<'a>)],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attrs: &'a [&'a XhpAttr<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub enum_: Option<&'a Enum_<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Class_<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Class_<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct XhpAttr<'a, Ex, Fb, En, Hi>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a TypeHint<'a, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a ClassVar<'a, Ex, Fb, En, Hi>,
    pub Option<oxidized::aast::XhpAttrTag>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  Option<&'a (&'a Pos<'a>, &'a [&'a Expr<'a, Ex, Fb, En, Hi>])>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for XhpAttr<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(XhpAttr<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum ClassAttr<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CAName(&'a Sid<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CAField(&'a CaField<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassAttr<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(ClassAttr<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct CaField<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: CaType<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub value: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub required: bool,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for CaField<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(CaField<'arena, Ex, Fb, En, Hi>);

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
pub enum CaType<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CAHint(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CAEnum(&'a [&'a str]),
}
impl<'a> TrivialDrop for CaType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CaType<'arena>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct ClassConst<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Sid<'a>,
    /// expr = None indicates an abstract const
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassConst<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(ClassConst<'arena, Ex, Fb, En, Hi>);

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
pub struct ClassConcreteTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub c_tc_type: &'a Hint<'a>,
}
impl<'a> TrivialDrop for ClassConcreteTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassConcreteTypeconst<'arena>);

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
pub struct ClassPartiallyAbstractTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraint: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Hint<'a>,
}
impl<'a> TrivialDrop for ClassPartiallyAbstractTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassPartiallyAbstractTypeconst<'arena>);

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
pub enum ClassTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TCAbstract(&'a ClassAbstractTypeconst<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TCConcrete(&'a ClassConcreteTypeconst<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TCPartiallyAbstract(&'a ClassPartiallyAbstractTypeconst<'a>),
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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct ClassTypeconstDef<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
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
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassTypeconstDef<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(ClassTypeconstDef<'arena, Ex, Fb, En, Hi>);

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
pub struct XhpAttrInfo<'a> {
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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct ClassVar<'a, Ex, Fb, En, Hi> {
    pub final_: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attr: Option<&'a XhpAttrInfo<'a>>,
    pub abstract_: bool,
    pub readonly: bool,
    pub visibility: oxidized::aast::Visibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a TypeHint<'a, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub id: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassVar<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(ClassVar<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Method_<'a, Ex, Fb, En, Hi> {
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
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub unsafe_ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub body: &'a FuncBody<'a, Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub readonly_ret: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ret: &'a TypeHint<'a, Hi>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Method_<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Method_<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Typedef<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub mode: oxidized::file_info::Mode,
    pub vis: oxidized::aast::TypedefVisibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Typedef<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Typedef<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct Gconst<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub value: &'a Expr<'a, Ex, Fb, En, Hi>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Gconst<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Gconst<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct RecordDef<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena")]
    pub annotation: En,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub extends: Option<&'a RecordHint<'a>>,
    pub abstract_: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fields: &'a [(Sid<'a>, &'a Hint<'a>, Option<&'a Expr<'a, Ex, Fb, En, Hi>>)],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub span: &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub doc_comment: Option<&'a DocComment<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for RecordDef<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(RecordDef<'arena, Ex, Fb, En, Hi>);

pub type RecordHint<'a> = Hint<'a>;

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct FunDef<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub namespace: &'a Nsenv<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, Fb, En, Hi>],
    pub mode: oxidized::file_info::Mode,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub fun: &'a Fun_<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunDef<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(FunDef<'arena, Ex, Fb, En, Hi>);

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
    deserialize = "Ex: 'de + arena_deserializer::DeserializeInArena<'de>, Fb: 'de + arena_deserializer::DeserializeInArena<'de>, En: 'de + arena_deserializer::DeserializeInArena<'de>, Hi: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub enum Def<'a, Ex, Fb, En, Hi> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Fun(&'a FunDef<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Class(&'a Class_<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RecordDef(&'a RecordDef<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Stmt(&'a Stmt<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Typedef(&'a Typedef<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Constant(&'a Gconst<'a, Ex, Fb, En, Hi>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Namespace(&'a (Sid<'a>, &'a Program<'a, Ex, Fb, En, Hi>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    NamespaceUse(&'a [(oxidized::aast::NsKind, Sid<'a>, Sid<'a>)]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    SetNamespaceEnv(&'a Nsenv<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FileAttributes(&'a FileAttribute<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Def<'a, Ex, Fb, En, Hi>
{
}
arena_deserializer::impl_deserialize_in_arena!(Def<'arena, Ex, Fb, En, Hi>);

pub use oxidized::aast::NsKind;

pub use oxidized::aast::HoleSource;

pub use oxidized::aast::BreakContinueLevel;
