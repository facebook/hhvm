// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<34f655b22c3c4d78e19d6a446e4a8fa9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
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
pub struct Stmt<'a, Ex, Fb, En, Hi>(pub &'a Pos<'a>, pub Stmt_<'a, Ex, Fb, En, Hi>);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Stmt<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum Stmt_<'a, Ex, Fb, En, Hi> {
    /// Marker for a switch statement that falls through.
    ///
    /// // FALLTHROUGH
    Fallthrough,
    /// Standalone expression.
    ///
    /// 1 + 2;
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
    Throw(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Return, with an optional value.
    ///
    /// return;
    /// return $foo;
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
    Awaitall(
        &'a (
            &'a [(Option<&'a Lid<'a>>, &'a Expr<'a, Ex, Fb, En, Hi>)],
            &'a Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// If statement.
    ///
    /// if ($foo) { ... } else { ... }
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
    Do(&'a (&'a Block<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// While loop.
    ///
    /// while ($foo) {
    /// bar();
    /// }
    While(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Block<'a, Ex, Fb, En, Hi>)),
    /// Initialize a value that is automatically disposed of.
    ///
    /// using $foo = bar(); // disposed at the end of the function
    /// using ($foo = bar(), $baz = quux()) {} // disposed after the block
    Using(&'a UsingStmt<'a, Ex, Fb, En, Hi>),
    /// For loop. The initializer and increment parts can include
    /// multiple comma-separated statements. The termination condition is
    /// optional.
    ///
    /// for ($i = 0; $i < 100; $i++) { ... }
    /// for ($x = 0, $y = 0; ; $x++, $y++) { ... }
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
    Switch(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a [Case<'a, Ex, Fb, En, Hi>])),
    /// For-each loop.
    ///
    /// foreach ($items as $item) { ... }
    /// foreach ($items as $key => value) { ... }
    /// foreach ($items await as $item) { ... } // AsyncIterator<_>
    /// foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_>
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
    Block(&'a Block<'a, Ex, Fb, En, Hi>),
    /// The mode tag at the beginning of a file.
    /// TODO: this really belongs in def.
    ///
    /// <?hh
    Markup(&'a Pstring<'a>),
    /// Used in IFC to track type inference environments. Not user
    /// denotable.
    AssertEnv(&'a (oxidized::aast::EnvAnnot, &'a LocalIdMap<'a, Ex>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Stmt_<'a, Ex, Fb, En, Hi>
{
}

pub use oxidized::aast::EnvAnnot;

#[derive(
    Clone,
    Debug,
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
pub struct UsingStmt<'a, Ex, Fb, En, Hi> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    pub exprs: (&'a Pos<'a>, &'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    pub block: &'a Block<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for UsingStmt<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum AsExpr<'a, Ex, Fb, En, Hi> {
    AsV(&'a Expr<'a, Ex, Fb, En, Hi>),
    AsKv(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    AwaitAsV(&'a (&'a Pos<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)),
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

pub type Block<'a, Ex, Fb, En, Hi> = [&'a Stmt<'a, Ex, Fb, En, Hi>];

#[derive(
    Clone,
    Debug,
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
pub struct ClassId<'a, Ex, Fb, En, Hi>(pub Ex, pub ClassId_<'a, Ex, Fb, En, Hi>);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassId<'a, Ex, Fb, En, Hi>
{
}

/// Class ID, used in things like instantiation and static property access.
#[derive(
    Clone,
    Copy,
    Debug,
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
    CIexpr(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Explicit class name. This is the common case.
    ///
    /// Foop::some_meth()
    /// Foo::$prop = 1;
    /// new Foo();
    CI(&'a Sid<'a>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassId_<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct Expr<'a, Ex, Fb, En, Hi>(pub Ex, pub Expr_<'a, Ex, Fb, En, Hi>);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Expr<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum CollectionTarg<'a, Hi> {
    CollectionTV(&'a Targ<'a, Hi>),
    CollectionTKV(&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)),
}
impl<'a, Hi: TrivialDrop> TrivialDrop for CollectionTarg<'a, Hi> {}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum FunctionPtrId<'a, Ex, Fb, En, Hi> {
    FPId(&'a Sid<'a>),
    FPClassConst(&'a (&'a ClassId<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunctionPtrId<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct ExpressionTree<'a, Ex, Fb, En, Hi> {
    pub hint: &'a Hint<'a>,
    pub splices: &'a Block<'a, Ex, Fb, En, Hi>,
    pub virtualized_expr: &'a Expr<'a, Ex, Fb, En, Hi>,
    pub runtime_expr: &'a Expr<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ExpressionTree<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum Expr_<'a, Ex, Fb, En, Hi> {
    /// darray literal.
    ///
    /// darray['x' => 0, 'y' => 1]
    /// darray<string, int>['x' => 0, 'y' => 1]
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
    Varray(&'a (Option<&'a Targ<'a, Hi>>, &'a [&'a Expr<'a, Ex, Fb, En, Hi>])),
    /// Shape literal.
    ///
    /// shape('x' => 1, 'y' => 2)
    Shape(&'a [(ast_defs::ShapeFieldName<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)]),
    /// Collection literal for indexable structures.
    ///
    /// Vector {1, 2}
    /// ImmVector {}
    /// Set<string> {'foo', 'bar'}
    /// vec[1, 2]
    /// keyset[]
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
    Id(&'a Sid<'a>),
    /// Local variable.
    ///
    /// $foo
    Lvar(&'a Lid<'a>),
    /// The extra variable in a pipe expression.
    ///
    /// $$
    Dollardollar(&'a Lid<'a>),
    /// Clone expression.
    ///
    /// clone $foo
    Clone(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Array indexing.
    ///
    /// $foo[]
    /// $foo[$bar]
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
    FunctionPointer(&'a (FunctionPtrId<'a, Ex, Fb, En, Hi>, &'a [&'a Targ<'a, Hi>])),
    /// Integer literal.
    ///
    /// 42
    /// 0123 // octal
    /// 0xBEEF // hexadecimal
    /// 0b11111111 // binary
    Int(&'a str),
    /// Float literal.
    ///
    /// 1.0
    /// 1.2e3
    /// 7E-10
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
    String(&'a bstr::BStr),
    /// Interpolated string literal.
    ///
    /// "hello $foo $bar"
    ///
    /// <<<DOC
    /// hello $foo $bar
    /// DOC
    String2(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    /// Prefixed string literal. Only used for regular expressions.
    ///
    /// re"foo"
    PrefixedString(&'a (&'a str, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Yield expression. The enclosing function should have an Iterator
    /// return type.
    ///
    /// yield $foo // enclosing function returns an Iterator
    /// yield $foo => $bar // enclosing function returns a KeyedIterator
    Yield(&'a Afield<'a, Ex, Fb, En, Hi>),
    /// Await expression.
    ///
    /// await $foo
    Await(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Readonly expression.
    ///
    /// readonly $foo
    ReadonlyExpr(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Tuple expression.
    ///
    /// tuple("a", 1, $foo)
    Tuple(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    /// List expression, only used in destructuring. Allows any arbitrary
    /// lvalue as a subexpression. May also nest.
    ///
    /// list($x, $y) = vec[1, 2];
    /// list(, $y) = vec[1, 2]; // skipping items
    /// list(list($x)) = vec[vec[1]]; // nesting
    /// list($v[0], $x[], $y->foo) = $blah;
    List(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    /// Cast expression, converting a value to a different type. Only
    /// primitive types are supported in the hint position.
    ///
    /// (int)$foo
    /// (string)$foo
    Cast(&'a (&'a Hint<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Unary operator.
    ///
    /// !$foo
    /// -$foo
    /// +$foo
    Unop(&'a (oxidized::ast_defs::Uop, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Binary operator.
    ///
    /// $foo + $bar
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
    Is(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Hint<'a>)),
    /// As operator.
    ///
    /// $foo as int
    /// $foo ?as int
    As(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Hint<'a>, bool)),
    /// Instantiation.
    ///
    /// new Foo(1, 2);
    /// new Foo<int, T>();
    /// new Foo('blah', ...$rest);
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
    Efun(&'a (&'a Fun_<'a, Ex, Fb, En, Hi>, &'a [&'a Lid<'a>])),
    /// Hack lambda. Captures variables automatically.
    ///
    /// $x ==> $x
    /// (int $x): int ==> $x + $other
    /// ($x, $y) ==> { return $x + $y; }
    Lfun(&'a (&'a Fun_<'a, Ex, Fb, En, Hi>, &'a [&'a Lid<'a>])),
    /// XHP expression. May contain interpolated expressions.
    ///
    /// <foo x="hello" y={$foo}>hello {$bar}</foo>
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
    Callconv(&'a (oxidized::ast_defs::ParamKind, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Include or require expression.
    ///
    /// require('foo.php')
    /// require_once('foo.php')
    /// include('foo.php')
    /// include_once('foo.php')
    Import(&'a (oxidized::aast::ImportFlavor, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// Collection literal.
    ///
    /// TODO: T38184446 this is redundant with ValCollection/KeyValCollection.
    ///
    /// Vector {}
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
    ExpressionTree(&'a ExpressionTree<'a, Ex, Fb, En, Hi>),
    /// Placeholder local variable.
    ///
    /// $_
    Lplaceholder(&'a Pos<'a>),
    /// Global function reference.
    ///
    /// fun('foo')
    FunId(&'a Sid<'a>),
    /// Instance method reference on a specific instance.
    ///
    /// TODO: This is only created in naming, and ought to happen in
    /// lowering or be removed. The emitter just sees a normal Call.
    ///
    /// inst_meth($f, 'some_meth') // equivalent: $f->some_meth<>
    MethodId(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
    /// Instance method reference that can be called with an instance.
    ///
    /// meth_caller(FooClass::class, 'some_meth')
    /// meth_caller('FooClass', 'some_meth')
    ///
    /// These examples are equivalent to:
    ///
    /// (FooClass $f, ...$args) ==> $f->some_meth(...$args)
    MethodCaller(&'a (Sid<'a>, &'a Pstring<'a>)),
    /// Static method reference.
    ///
    /// class_meth('FooClass', 'some_static_meth')
    /// // equivalent: FooClass::some_static_meth<>
    SmethodId(&'a (&'a ClassId<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
    /// Pair literal.
    ///
    /// Pair {$foo, $bar}
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
    ETSplice(&'a Expr<'a, Ex, Fb, En, Hi>),
    /// Enum atom used for enum classes.
    ///
    /// #field_name
    EnumAtom(&'a str),
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

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum ClassGetExpr<'a, Ex, Fb, En, Hi> {
    CGstring(&'a Pstring<'a>),
    CGexpr(&'a Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassGetExpr<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum Case<'a, Ex, Fb, En, Hi> {
    Default(&'a (&'a Pos<'a>, &'a Block<'a, Ex, Fb, En, Hi>)),
    Case(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Block<'a, Ex, Fb, En, Hi>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Case<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct Catch<'a, Ex, Fb, En, Hi>(
    pub Sid<'a>,
    pub &'a Lid<'a>,
    pub &'a Block<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Catch<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct Field<'a, Ex, Fb, En, Hi>(
    pub &'a Expr<'a, Ex, Fb, En, Hi>,
    pub &'a Expr<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Field<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum Afield<'a, Ex, Fb, En, Hi> {
    AFvalue(&'a Expr<'a, Ex, Fb, En, Hi>),
    AFkvalue(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Afield<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct XhpSimple<'a, Ex, Fb, En, Hi> {
    pub name: &'a Pstring<'a>,
    pub type_: Hi,
    pub expr: &'a Expr<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for XhpSimple<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum XhpAttribute<'a, Ex, Fb, En, Hi> {
    XhpSimple(&'a XhpSimple<'a, Ex, Fb, En, Hi>),
    XhpSpread(&'a Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for XhpAttribute<'a, Ex, Fb, En, Hi>
{
}

pub use oxidized::aast::IsVariadic;

#[derive(
    Clone,
    Debug,
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
pub struct FunParam<'a, Ex, Fb, En, Hi> {
    pub annotation: Ex,
    pub type_hint: &'a TypeHint<'a, Hi>,
    pub is_variadic: &'a oxidized::aast::IsVariadic,
    pub pos: &'a Pos<'a>,
    pub name: &'a str,
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub readonly: Option<oxidized::ast_defs::ReadonlyKind>,
    pub callconv: Option<oxidized::ast_defs::ParamKind>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub visibility: Option<oxidized::aast::Visibility>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunParam<'a, Ex, Fb, En, Hi>
{
}

/// Does this function/method take a variable number of arguments?
#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum FunVariadicity<'a, Ex, Fb, En, Hi> {
    /// Named variadic argument.
    ///
    /// function foo(int ...$args): void {}
    FVvariadicArg(&'a FunParam<'a, Ex, Fb, En, Hi>),
    /// Unnamed variaidic argument. Partial mode only.
    ///
    /// function foo(...): void {}
    FVellipsis(&'a Pos<'a>),
    /// Function is not variadic, takes an exact number of arguments.
    FVnonVariadic,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunVariadicity<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct Fun_<'a, Ex, Fb, En, Hi> {
    pub span: &'a Pos<'a>,
    pub readonly_this: Option<oxidized::ast_defs::ReadonlyKind>,
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub readonly_ret: Option<oxidized::ast_defs::ReadonlyKind>,
    pub ret: &'a TypeHint<'a, Hi>,
    pub name: Sid<'a>,
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    pub ctxs: Option<&'a Contexts<'a>>,
    pub unsafe_ctxs: Option<&'a Contexts<'a>>,
    pub body: &'a FuncBody<'a, Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, Fb, En, Hi>],
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    pub namespace: &'a Nsenv<'a>,
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Fun_<'a, Ex, Fb, En, Hi>
{
}

/// Naming has two phases and the annotation helps to indicate the phase.
/// In the first pass, it will perform naming on everything except for function
/// and method bodies and collect information needed. Then, another round of
/// naming is performed where function bodies are named. Thus, naming will
/// have named and unnamed variants of the annotation.
/// See BodyNamingAnnotation in nast.ml and the comment in naming.ml
#[derive(
    Clone,
    Debug,
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
pub struct FuncBody<'a, Ex, Fb, En, Hi> {
    pub ast: &'a Block<'a, Ex, Fb, En, Hi>,
    pub annotation: Fb,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FuncBody<'a, Ex, Fb, En, Hi>
{
}

/// A type annotation is two things:
/// - the localized hint, or if the hint is missing, the inferred type
/// - The typehint associated to this expression if it exists
#[derive(
    Clone,
    Debug,
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
pub struct TypeHint<'a, Hi>(pub Hi, pub &'a TypeHint_<'a>);
impl<'a, Hi: TrivialDrop> TrivialDrop for TypeHint<'a, Hi> {}

/// Explicit type argument to function, constructor, or collection literal.
/// 'hi = unit in NAST
/// 'hi = Typing_defs.(locl ty) in TAST,
/// and is used to record inferred type arguments, with wildcard hint.
#[derive(
    Clone,
    Debug,
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
pub struct Targ<'a, Hi>(pub Hi, pub &'a Hint<'a>);
impl<'a, Hi: TrivialDrop> TrivialDrop for Targ<'a, Hi> {}

pub type TypeHint_<'a> = Option<&'a Hint<'a>>;

#[derive(
    Clone,
    Debug,
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
pub struct UserAttribute<'a, Ex, Fb, En, Hi> {
    pub name: Sid<'a>,
    /// user attributes are restricted to scalar values
    pub params: &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for UserAttribute<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct FileAttribute<'a, Ex, Fb, En, Hi> {
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub namespace: &'a Nsenv<'a>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FileAttribute<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct Tparam<'a, Ex, Fb, En, Hi> {
    pub variance: oxidized::ast_defs::Variance,
    pub name: Sid<'a>,
    pub parameters: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub constraints: &'a [(oxidized::ast_defs::ConstraintKind, &'a Hint<'a>)],
    pub reified: oxidized::aast::ReifyKind,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Tparam<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
    pub Option<Sid<'a>>,
    pub &'a Pstring<'a>,
    pub Option<Sid<'a>>,
    pub &'a [oxidized::aast::UseAsVisibility],
);
impl<'a> TrivialDrop for UseAsAlias<'a> {}

#[derive(
    Clone,
    Debug,
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
pub struct InsteadofAlias<'a>(pub Sid<'a>, pub &'a Pstring<'a>, pub &'a [Sid<'a>]);
impl<'a> TrivialDrop for InsteadofAlias<'a> {}

pub use oxidized::aast::IsExtends;

pub use oxidized::aast::EmitId;

#[derive(
    Clone,
    Debug,
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
pub struct Class_<'a, Ex, Fb, En, Hi> {
    pub span: &'a Pos<'a>,
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassKind,
    pub name: Sid<'a>,
    /// The type parameters of a class A<T> (T is the parameter)
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub extends: &'a [&'a ClassHint<'a>],
    pub uses: &'a [&'a TraitHint<'a>],
    /// PHP feature not supported in hack but required
    /// because we have runtime support.
    pub use_as_alias: &'a [&'a UseAsAlias<'a>],
    /// PHP feature not supported in hack but required
    /// because we have runtime support.
    pub insteadof_alias: &'a [&'a InsteadofAlias<'a>],
    pub xhp_attr_uses: &'a [&'a XhpAttrHint<'a>],
    pub xhp_category: Option<&'a (&'a Pos<'a>, &'a [&'a Pstring<'a>])>,
    pub reqs: &'a [(&'a ClassHint<'a>, &'a oxidized::aast::IsExtends)],
    pub implements: &'a [&'a ClassHint<'a>],
    pub implements_dynamic: bool,
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    pub consts: &'a [&'a ClassConst<'a, Ex, Fb, En, Hi>],
    pub typeconsts: &'a [&'a ClassTypeconstDef<'a, Ex, Fb, En, Hi>],
    pub vars: &'a [&'a ClassVar<'a, Ex, Fb, En, Hi>],
    pub methods: &'a [&'a Method_<'a, Ex, Fb, En, Hi>],
    pub attributes: &'a [ClassAttr<'a, Ex, Fb, En, Hi>],
    pub xhp_children: &'a [(&'a Pos<'a>, &'a XhpChild<'a>)],
    pub xhp_attrs: &'a [&'a XhpAttr<'a, Ex, Fb, En, Hi>],
    pub namespace: &'a Nsenv<'a>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, Fb, En, Hi>],
    pub enum_: Option<&'a Enum_<'a>>,
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Class_<'a, Ex, Fb, En, Hi>
{
}

pub type ClassHint<'a> = Hint<'a>;

pub type TraitHint<'a> = Hint<'a>;

pub type XhpAttrHint<'a> = Hint<'a>;

pub use oxidized::aast::XhpAttrTag;

#[derive(
    Clone,
    Debug,
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
pub struct XhpAttr<'a, Ex, Fb, En, Hi>(
    pub &'a TypeHint<'a, Hi>,
    pub &'a ClassVar<'a, Ex, Fb, En, Hi>,
    pub Option<oxidized::aast::XhpAttrTag>,
    pub Option<&'a (&'a Pos<'a>, &'a [&'a Expr<'a, Ex, Fb, En, Hi>])>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for XhpAttr<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum ClassAttr<'a, Ex, Fb, En, Hi> {
    CAName(&'a Sid<'a>),
    CAField(&'a CaField<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassAttr<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct CaField<'a, Ex, Fb, En, Hi> {
    pub type_: CaType<'a>,
    pub id: Sid<'a>,
    pub value: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub required: bool,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for CaField<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Copy,
    Debug,
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
    CAHint(&'a Hint<'a>),
    CAEnum(&'a [&'a str]),
}
impl<'a> TrivialDrop for CaType<'a> {}

#[derive(
    Clone,
    Debug,
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
pub struct ClassConst<'a, Ex, Fb, En, Hi> {
    pub type_: Option<&'a Hint<'a>>,
    pub id: Sid<'a>,
    /// expr = None indicates an abstract const
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassConst<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
    pub as_constraint: Option<&'a Hint<'a>>,
    pub super_constraint: Option<&'a Hint<'a>>,
    pub default: Option<&'a Hint<'a>>,
}
impl<'a> TrivialDrop for ClassAbstractTypeconst<'a> {}

#[derive(
    Clone,
    Debug,
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
    pub c_tc_type: &'a Hint<'a>,
}
impl<'a> TrivialDrop for ClassConcreteTypeconst<'a> {}

#[derive(
    Clone,
    Debug,
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
    pub constraint: &'a Hint<'a>,
    pub type_: &'a Hint<'a>,
}
impl<'a> TrivialDrop for ClassPartiallyAbstractTypeconst<'a> {}

#[derive(
    Clone,
    Copy,
    Debug,
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
    TCAbstract(&'a ClassAbstractTypeconst<'a>),
    TCConcrete(&'a ClassConcreteTypeconst<'a>),
    TCPartiallyAbstract(&'a ClassPartiallyAbstractTypeconst<'a>),
}
impl<'a> TrivialDrop for ClassTypeconst<'a> {}

#[derive(
    Clone,
    Debug,
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
pub struct ClassTypeconstDef<'a, Ex, Fb, En, Hi> {
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub name: Sid<'a>,
    pub kind: ClassTypeconst<'a>,
    pub span: &'a Pos<'a>,
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub is_ctx: bool,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassTypeconstDef<'a, Ex, Fb, En, Hi>
{
}

pub use oxidized::aast::XhpAttrInfo;

#[derive(
    Clone,
    Debug,
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
pub struct ClassVar<'a, Ex, Fb, En, Hi> {
    pub final_: bool,
    pub xhp_attr: Option<&'a oxidized::aast::XhpAttrInfo>,
    pub abstract_: bool,
    pub readonly: bool,
    pub visibility: oxidized::aast::Visibility,
    pub type_: &'a TypeHint<'a, Hi>,
    pub id: Sid<'a>,
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    pub span: &'a Pos<'a>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassVar<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct Method_<'a, Ex, Fb, En, Hi> {
    pub span: &'a Pos<'a>,
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub readonly_this: bool,
    pub visibility: oxidized::aast::Visibility,
    pub name: Sid<'a>,
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    pub ctxs: Option<&'a Contexts<'a>>,
    pub unsafe_ctxs: Option<&'a Contexts<'a>>,
    pub body: &'a FuncBody<'a, Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub readonly_ret: Option<oxidized::ast_defs::ReadonlyKind>,
    pub ret: &'a TypeHint<'a, Hi>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Method_<'a, Ex, Fb, En, Hi>
{
}

pub type Nsenv<'a> = namespace_env::Env<'a>;

#[derive(
    Clone,
    Debug,
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
pub struct Typedef<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid<'a>,
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub constraint: Option<&'a Hint<'a>>,
    pub kind: &'a Hint<'a>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub mode: oxidized::file_info::Mode,
    pub vis: oxidized::aast::TypedefVisibility,
    pub namespace: &'a Nsenv<'a>,
    pub span: &'a Pos<'a>,
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Typedef<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct Gconst<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub name: Sid<'a>,
    pub type_: Option<&'a Hint<'a>>,
    pub value: &'a Expr<'a, Ex, Fb, En, Hi>,
    pub namespace: &'a Nsenv<'a>,
    pub span: &'a Pos<'a>,
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Gconst<'a, Ex, Fb, En, Hi>
{
}

#[derive(
    Clone,
    Debug,
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
pub struct RecordDef<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid<'a>,
    pub extends: Option<&'a RecordHint<'a>>,
    pub abstract_: bool,
    pub fields: &'a [(Sid<'a>, &'a Hint<'a>, Option<&'a Expr<'a, Ex, Fb, En, Hi>>)],
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub namespace: &'a Nsenv<'a>,
    pub span: &'a Pos<'a>,
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for RecordDef<'a, Ex, Fb, En, Hi>
{
}

pub type RecordHint<'a> = Hint<'a>;

pub type FunDef<'a, Ex, Fb, En, Hi> = Fun_<'a, Ex, Fb, En, Hi>;

#[derive(
    Clone,
    Copy,
    Debug,
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
pub enum Def<'a, Ex, Fb, En, Hi> {
    Fun(&'a FunDef<'a, Ex, Fb, En, Hi>),
    Class(&'a Class_<'a, Ex, Fb, En, Hi>),
    RecordDef(&'a RecordDef<'a, Ex, Fb, En, Hi>),
    Stmt(&'a Stmt<'a, Ex, Fb, En, Hi>),
    Typedef(&'a Typedef<'a, Ex, Fb, En, Hi>),
    Constant(&'a Gconst<'a, Ex, Fb, En, Hi>),
    Namespace(&'a (Sid<'a>, &'a Program<'a, Ex, Fb, En, Hi>)),
    NamespaceUse(&'a [(oxidized::aast::NsKind, Sid<'a>, Sid<'a>)]),
    SetNamespaceEnv(&'a Nsenv<'a>),
    FileAttributes(&'a FileAttribute<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Def<'a, Ex, Fb, En, Hi>
{
}

pub use oxidized::aast::NsKind;

pub use oxidized::aast::HoleSource;

pub use oxidized::aast::BreakContinueLevel;
