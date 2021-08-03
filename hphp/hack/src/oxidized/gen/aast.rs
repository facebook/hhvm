// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<342d54a77443c0686ea6c60dc31ebcdb>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
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
pub type Program<Ex, Fb, En> = Vec<Def<Ex, Fb, En>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Stmt<Ex, Fb, En>(pub Pos, pub Stmt_<Ex, Fb, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Stmt_<Ex, Fb, En> {
    /// Marker for a switch statement that falls through.
    ///
    /// // FALLTHROUGH
    Fallthrough,
    /// Standalone expression.
    ///
    /// 1 + 2;
    Expr(Box<Expr<Ex, Fb, En>>),
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
    Throw(Box<Expr<Ex, Fb, En>>),
    /// Return, with an optional value.
    ///
    /// return;
    /// return $foo;
    Return(Box<Option<Expr<Ex, Fb, En>>>),
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
    Awaitall(Box<(Vec<(Option<Lid>, Expr<Ex, Fb, En>)>, Block<Ex, Fb, En>)>),
    /// If statement.
    ///
    /// if ($foo) { ... } else { ... }
    If(Box<(Expr<Ex, Fb, En>, Block<Ex, Fb, En>, Block<Ex, Fb, En>)>),
    /// Do-while loop.
    ///
    /// do {
    /// bar();
    /// } while($foo)
    Do(Box<(Block<Ex, Fb, En>, Expr<Ex, Fb, En>)>),
    /// While loop.
    ///
    /// while ($foo) {
    /// bar();
    /// }
    While(Box<(Expr<Ex, Fb, En>, Block<Ex, Fb, En>)>),
    /// Initialize a value that is automatically disposed of.
    ///
    /// using $foo = bar(); // disposed at the end of the function
    /// using ($foo = bar(), $baz = quux()) {} // disposed after the block
    Using(Box<UsingStmt<Ex, Fb, En>>),
    /// For loop. The initializer and increment parts can include
    /// multiple comma-separated statements. The termination condition is
    /// optional.
    ///
    /// for ($i = 0; $i < 100; $i++) { ... }
    /// for ($x = 0, $y = 0; ; $x++, $y++) { ... }
    For(
        Box<(
            Vec<Expr<Ex, Fb, En>>,
            Option<Expr<Ex, Fb, En>>,
            Vec<Expr<Ex, Fb, En>>,
            Block<Ex, Fb, En>,
        )>,
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
    Switch(Box<(Expr<Ex, Fb, En>, Vec<Case<Ex, Fb, En>>)>),
    /// For-each loop.
    ///
    /// foreach ($items as $item) { ... }
    /// foreach ($items as $key => value) { ... }
    /// foreach ($items await as $item) { ... } // AsyncIterator<_>
    /// foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_>
    Foreach(Box<(Expr<Ex, Fb, En>, AsExpr<Ex, Fb, En>, Block<Ex, Fb, En>)>),
    /// Try statement, with catch blocks and a finally block.
    ///
    /// try {
    /// foo();
    /// } catch (SomeException $e) {
    /// bar();
    /// } finally {
    /// baz();
    /// }
    Try(Box<(Block<Ex, Fb, En>, Vec<Catch<Ex, Fb, En>>, Block<Ex, Fb, En>)>),
    /// No-op, the empty statement.
    ///
    /// {}
    /// while (true) ;
    /// if ($foo) {} // the else is Noop here
    Noop,
    /// Block, a list of statements in curly braces.
    ///
    /// { $foo = 42; }
    Block(Block<Ex, Fb, En>),
    /// The mode tag at the beginning of a file.
    /// TODO: this really belongs in def.
    ///
    /// <?hh
    Markup(Box<Pstring>),
    /// Used in IFC to track type inference environments. Not user
    /// denotable.
    AssertEnv(Box<(EnvAnnot, LocalIdMap<(Pos, Ex)>)>),
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum EnvAnnot {
    Join,
    Refinement,
}
impl TrivialDrop for EnvAnnot {}
arena_deserializer::impl_deserialize_in_arena!(EnvAnnot);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct UsingStmt<Ex, Fb, En> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    pub exprs: (Pos, Vec<Expr<Ex, Fb, En>>),
    pub block: Block<Ex, Fb, En>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum AsExpr<Ex, Fb, En> {
    AsV(Expr<Ex, Fb, En>),
    AsKv(Expr<Ex, Fb, En>, Expr<Ex, Fb, En>),
    AwaitAsV(Pos, Expr<Ex, Fb, En>),
    AwaitAsKv(Pos, Expr<Ex, Fb, En>, Expr<Ex, Fb, En>),
}

pub type Block<Ex, Fb, En> = Vec<Stmt<Ex, Fb, En>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ClassId<Ex, Fb, En>(pub Ex, pub Pos, pub ClassId_<Ex, Fb, En>);

/// Class ID, used in things like instantiation and static property access.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum ClassId_<Ex, Fb, En> {
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
    CIexpr(Expr<Ex, Fb, En>),
    /// Explicit class name. This is the common case.
    ///
    /// Foop::some_meth()
    /// Foo::$prop = 1;
    /// new Foo();
    CI(Sid),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Expr<Ex, Fb, En>(pub Ex, pub Pos, pub Expr_<Ex, Fb, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum CollectionTarg<Ex> {
    CollectionTV(Targ<Ex>),
    CollectionTKV(Targ<Ex>, Targ<Ex>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum FunctionPtrId<Ex, Fb, En> {
    FPId(Sid),
    FPClassConst(ClassId<Ex, Fb, En>, Pstring),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ExpressionTree<Ex, Fb, En> {
    pub hint: Hint,
    pub splices: Block<Ex, Fb, En>,
    pub virtualized_expr: Expr<Ex, Fb, En>,
    pub runtime_expr: Expr<Ex, Fb, En>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Expr_<Ex, Fb, En> {
    /// darray literal.
    ///
    /// darray['x' => 0, 'y' => 1]
    /// darray<string, int>['x' => 0, 'y' => 1]
    Darray(
        Box<(
            Option<(Targ<Ex>, Targ<Ex>)>,
            Vec<(Expr<Ex, Fb, En>, Expr<Ex, Fb, En>)>,
        )>,
    ),
    /// varray literal.
    ///
    /// varray['hello', 'world']
    /// varray<string>['hello', 'world']
    Varray(Box<(Option<Targ<Ex>>, Vec<Expr<Ex, Fb, En>>)>),
    /// Shape literal.
    ///
    /// shape('x' => 1, 'y' => 2)
    Shape(Vec<(ast_defs::ShapeFieldName, Expr<Ex, Fb, En>)>),
    /// Collection literal for indexable structures.
    ///
    /// Vector {1, 2}
    /// ImmVector {}
    /// Set<string> {'foo', 'bar'}
    /// vec[1, 2]
    /// keyset[]
    ValCollection(Box<(VcKind, Option<Targ<Ex>>, Vec<Expr<Ex, Fb, En>>)>),
    /// Collection literal for key-value structures.
    ///
    /// dict['x' => 1, 'y' => 2]
    /// Map<int, string> {}
    /// ImmMap {}
    KeyValCollection(
        Box<(
            KvcKind,
            Option<(Targ<Ex>, Targ<Ex>)>,
            Vec<Field<Ex, Fb, En>>,
        )>,
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
    Id(Box<Sid>),
    /// Local variable.
    ///
    /// $foo
    Lvar(Box<Lid>),
    /// The extra variable in a pipe expression.
    ///
    /// $$
    Dollardollar(Box<Lid>),
    /// Clone expression.
    ///
    /// clone $foo
    Clone(Box<Expr<Ex, Fb, En>>),
    /// Array indexing.
    ///
    /// $foo[]
    /// $foo[$bar]
    ArrayGet(Box<(Expr<Ex, Fb, En>, Option<Expr<Ex, Fb, En>>)>),
    /// Instance property or method access.  is_prop_call is always
    /// false, except when inside a call is accessing a property.
    ///
    /// $foo->bar // (Obj_get false) property access
    /// $foo->bar() // (Call (Obj_get false)) method call
    /// ($foo->bar)() // (Call (Obj_get true)) call lambda stored in property
    /// $foo?->bar // nullsafe access
    ObjGet(Box<(Expr<Ex, Fb, En>, Expr<Ex, Fb, En>, OgNullFlavor, bool)>),
    /// Static property access.
    ///
    /// Foo::$bar
    /// $some_classname::$bar
    /// Foo::${$bar} // only in partial mode
    ClassGet(Box<(ClassId<Ex, Fb, En>, ClassGetExpr<Ex, Fb, En>, bool)>),
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
    ClassConst(Box<(ClassId<Ex, Fb, En>, Pstring)>),
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
        Box<(
            Expr<Ex, Fb, En>,
            Vec<Targ<Ex>>,
            Vec<Expr<Ex, Fb, En>>,
            Option<Expr<Ex, Fb, En>>,
        )>,
    ),
    /// A reference to a function or method.
    ///
    /// foo_fun<>
    /// FooCls::meth<int>
    FunctionPointer(Box<(FunctionPtrId<Ex, Fb, En>, Vec<Targ<Ex>>)>),
    /// Integer literal.
    ///
    /// 42
    /// 0123 // octal
    /// 0xBEEF // hexadecimal
    /// 0b11111111 // binary
    Int(String),
    /// Float literal.
    ///
    /// 1.0
    /// 1.2e3
    /// 7E-10
    Float(String),
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
    String(bstr::BString),
    /// Interpolated string literal.
    ///
    /// "hello $foo $bar"
    ///
    /// <<<DOC
    /// hello $foo $bar
    /// DOC
    String2(Vec<Expr<Ex, Fb, En>>),
    /// Prefixed string literal. Only used for regular expressions.
    ///
    /// re"foo"
    PrefixedString(Box<(String, Expr<Ex, Fb, En>)>),
    /// Yield expression. The enclosing function should have an Iterator
    /// return type.
    ///
    /// yield $foo // enclosing function returns an Iterator
    /// yield $foo => $bar // enclosing function returns a KeyedIterator
    Yield(Box<Afield<Ex, Fb, En>>),
    /// Await expression.
    ///
    /// await $foo
    Await(Box<Expr<Ex, Fb, En>>),
    /// Readonly expression.
    ///
    /// readonly $foo
    ReadonlyExpr(Box<Expr<Ex, Fb, En>>),
    /// Tuple expression.
    ///
    /// tuple("a", 1, $foo)
    Tuple(Vec<Expr<Ex, Fb, En>>),
    /// List expression, only used in destructuring. Allows any arbitrary
    /// lvalue as a subexpression. May also nest.
    ///
    /// list($x, $y) = vec[1, 2];
    /// list(, $y) = vec[1, 2]; // skipping items
    /// list(list($x)) = vec[vec[1]]; // nesting
    /// list($v[0], $x[], $y->foo) = $blah;
    List(Vec<Expr<Ex, Fb, En>>),
    /// Cast expression, converting a value to a different type. Only
    /// primitive types are supported in the hint position.
    ///
    /// (int)$foo
    /// (string)$foo
    Cast(Box<(Hint, Expr<Ex, Fb, En>)>),
    /// Unary operator.
    ///
    /// !$foo
    /// -$foo
    /// +$foo
    Unop(Box<(ast_defs::Uop, Expr<Ex, Fb, En>)>),
    /// Binary operator.
    ///
    /// $foo + $bar
    Binop(Box<(ast_defs::Bop, Expr<Ex, Fb, En>, Expr<Ex, Fb, En>)>),
    /// Pipe expression. The lid is the ID of the $$ that is implicitly
    /// declared by this pipe.
    ///
    /// See also Dollardollar.
    ///
    /// $foo |> bar() // equivalent: bar($foo)
    /// $foo |> bar(1, $$) // equivalent: bar(1, $foo)
    Pipe(Box<(Lid, Expr<Ex, Fb, En>, Expr<Ex, Fb, En>)>),
    /// Ternary operator, or elvis operator.
    ///
    /// $foo ? $bar : $baz // ternary
    /// $foo ?: $baz // elvis
    Eif(Box<(Expr<Ex, Fb, En>, Option<Expr<Ex, Fb, En>>, Expr<Ex, Fb, En>)>),
    /// Is operator.
    ///
    /// $foo is SomeType
    Is(Box<(Expr<Ex, Fb, En>, Hint)>),
    /// As operator.
    ///
    /// $foo as int
    /// $foo ?as int
    As(Box<(Expr<Ex, Fb, En>, Hint, bool)>),
    /// Instantiation.
    ///
    /// new Foo(1, 2);
    /// new Foo<int, T>();
    /// new Foo('blah', ...$rest);
    New(
        Box<(
            ClassId<Ex, Fb, En>,
            Vec<Targ<Ex>>,
            Vec<Expr<Ex, Fb, En>>,
            Option<Expr<Ex, Fb, En>>,
            Ex,
        )>,
    ),
    /// Record literal.
    ///
    /// MyRecord['x' => $foo, 'y' => $bar]
    Record(Box<(Sid, Vec<(Expr<Ex, Fb, En>, Expr<Ex, Fb, En>)>)>),
    /// PHP-style lambda. Does not capture variables unless explicitly
    /// specified.
    ///
    /// Mnemonic: 'expanded lambda', since we can desugar Lfun to Efun.
    ///
    /// function($x) { return $x; }
    /// function(int $x): int { return $x; }
    /// function($x) use ($y) { return $y; }
    /// function($x): int use ($y, $z) { return $x + $y + $z; }
    Efun(Box<(Fun_<Ex, Fb, En>, Vec<Lid>)>),
    /// Hack lambda. Captures variables automatically.
    ///
    /// $x ==> $x
    /// (int $x): int ==> $x + $other
    /// ($x, $y) ==> { return $x + $y; }
    Lfun(Box<(Fun_<Ex, Fb, En>, Vec<Lid>)>),
    /// XHP expression. May contain interpolated expressions.
    ///
    /// <foo x="hello" y={$foo}>hello {$bar}</foo>
    Xml(Box<(Sid, Vec<XhpAttribute<Ex, Fb, En>>, Vec<Expr<Ex, Fb, En>>)>),
    /// Explicit calling convention, used for inout. Inout supports any lvalue.
    ///
    /// TODO: This could be a flag on parameters in Call.
    ///
    /// foo(inout $x[0])
    Callconv(Box<(ast_defs::ParamKind, Expr<Ex, Fb, En>)>),
    /// Include or require expression.
    ///
    /// require('foo.php')
    /// require_once('foo.php')
    /// include('foo.php')
    /// include_once('foo.php')
    Import(Box<(ImportFlavor, Expr<Ex, Fb, En>)>),
    /// Collection literal.
    ///
    /// TODO: T38184446 this is redundant with ValCollection/KeyValCollection.
    ///
    /// Vector {}
    Collection(Box<(Sid, Option<CollectionTarg<Ex>>, Vec<Afield<Ex, Fb, En>>)>),
    /// Expression tree literal. Expression trees are not evaluated at
    /// runtime, but desugared to an expression representing the code.
    ///
    /// Foo`1 + bar()`
    /// Foo`$x ==> $x * ${$value}` // splicing $value
    ExpressionTree(Box<ExpressionTree<Ex, Fb, En>>),
    /// Placeholder local variable.
    ///
    /// $_
    Lplaceholder(Box<Pos>),
    /// Global function reference.
    ///
    /// fun('foo')
    FunId(Box<Sid>),
    /// Instance method reference on a specific instance.
    ///
    /// TODO: This is only created in naming, and ought to happen in
    /// lowering or be removed. The emitter just sees a normal Call.
    ///
    /// inst_meth($f, 'some_meth') // equivalent: $f->some_meth<>
    MethodId(Box<(Expr<Ex, Fb, En>, Pstring)>),
    /// Instance method reference that can be called with an instance.
    ///
    /// meth_caller(FooClass::class, 'some_meth')
    /// meth_caller('FooClass', 'some_meth')
    ///
    /// These examples are equivalent to:
    ///
    /// (FooClass $f, ...$args) ==> $f->some_meth(...$args)
    MethodCaller(Box<(Sid, Pstring)>),
    /// Static method reference.
    ///
    /// class_meth('FooClass', 'some_static_meth')
    /// // equivalent: FooClass::some_static_meth<>
    SmethodId(Box<(ClassId<Ex, Fb, En>, Pstring)>),
    /// Pair literal.
    ///
    /// Pair {$foo, $bar}
    Pair(
        Box<(
            Option<(Targ<Ex>, Targ<Ex>)>,
            Expr<Ex, Fb, En>,
            Expr<Ex, Fb, En>,
        )>,
    ),
    /// Expression tree splice expression. Only valid inside an
    /// expression tree literal (backticks).
    ///
    /// ${$foo}
    ETSplice(Box<Expr<Ex, Fb, En>>),
    /// Label used for enum classes.
    ///
    /// enum_name#label_name or #label_name
    EnumClassLabel(Box<(Option<Sid>, String)>),
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
    Hole(Box<(Expr<Ex, Fb, En>, Ex, Ex, HoleSource)>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum HoleSource {
    Typing,
    UnsafeCast(Vec<Hint>),
    EnforcedCast(Vec<Hint>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum ClassGetExpr<Ex, Fb, En> {
    CGstring(Pstring),
    CGexpr(Expr<Ex, Fb, En>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Case<Ex, Fb, En> {
    Default(Pos, Block<Ex, Fb, En>),
    Case(Expr<Ex, Fb, En>, Block<Ex, Fb, En>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Catch<Ex, Fb, En>(pub Sid, pub Lid, pub Block<Ex, Fb, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Field<Ex, Fb, En>(pub Expr<Ex, Fb, En>, pub Expr<Ex, Fb, En>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Afield<Ex, Fb, En> {
    AFvalue(Expr<Ex, Fb, En>),
    AFkvalue(Expr<Ex, Fb, En>, Expr<Ex, Fb, En>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct XhpSimple<Ex, Fb, En> {
    pub name: Pstring,
    pub type_: Ex,
    pub expr: Expr<Ex, Fb, En>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum XhpAttribute<Ex, Fb, En> {
    XhpSimple(XhpSimple<Ex, Fb, En>),
    XhpSpread(Expr<Ex, Fb, En>),
}

pub type IsVariadic = bool;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct FunParam<Ex, Fb, En> {
    pub annotation: Ex,
    pub type_hint: TypeHint<Ex>,
    pub is_variadic: IsVariadic,
    pub pos: Pos,
    pub name: String,
    pub expr: Option<Expr<Ex, Fb, En>>,
    pub readonly: Option<ast_defs::ReadonlyKind>,
    pub callconv: Option<ast_defs::ParamKind>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    pub visibility: Option<Visibility>,
}

/// Does this function/method take a variable number of arguments?
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum FunVariadicity<Ex, Fb, En> {
    /// Named variadic argument.
    ///
    /// function foo(int ...$args): void {}
    FVvariadicArg(FunParam<Ex, Fb, En>),
    /// Unnamed variaidic argument. Partial mode only.
    ///
    /// function foo(...): void {}
    FVellipsis(Pos),
    /// Function is not variadic, takes an exact number of arguments.
    FVnonVariadic,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Fun_<Ex, Fb, En> {
    pub span: Pos,
    pub readonly_this: Option<ast_defs::ReadonlyKind>,
    pub annotation: En,
    pub readonly_ret: Option<ast_defs::ReadonlyKind>,
    pub ret: TypeHint<Ex>,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En>>,
    pub where_constraints: Vec<WhereConstraintHint>,
    pub variadic: FunVariadicity<Ex, Fb, En>,
    pub params: Vec<FunParam<Ex, Fb, En>>,
    pub ctxs: Option<Contexts>,
    pub unsafe_ctxs: Option<Contexts>,
    pub body: FuncBody<Ex, Fb, En>,
    pub fun_kind: ast_defs::FunKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<DocComment>,
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
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct FuncBody<Ex, Fb, En> {
    pub ast: Block<Ex, Fb, En>,
    pub annotation: Fb,
}

/// A type annotation is two things:
/// - the localized hint, or if the hint is missing, the inferred type
/// - The typehint associated to this expression if it exists
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct TypeHint<Ex>(pub Ex, pub TypeHint_);

/// Explicit type argument to function, constructor, or collection literal.
/// 'ex = unit in NAST
/// 'ex = Typing_defs.(locl ty) in TAST,
/// and is used to record inferred type arguments, with wildcard hint.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Targ<Ex>(pub Ex, pub Hint);

pub type TypeHint_ = Option<Hint>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct UserAttribute<Ex, Fb, En> {
    pub name: Sid,
    /// user attributes are restricted to scalar values
    pub params: Vec<Expr<Ex, Fb, En>>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct FileAttribute<Ex, Fb, En> {
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    pub namespace: Nsenv,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Tparam<Ex, Fb, En> {
    pub variance: ast_defs::Variance,
    pub name: Sid,
    pub parameters: Vec<Tparam<Ex, Fb, En>>,
    pub constraints: Vec<(ast_defs::ConstraintKind, Hint)>,
    pub reified: ReifyKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct UseAsAlias(
    pub Option<Sid>,
    pub Pstring,
    pub Option<Sid>,
    pub Vec<UseAsVisibility>,
);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct InsteadofAlias(pub Sid, pub Pstring, pub Vec<Sid>);

pub type IsExtends = bool;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum EmitId {
    EmitId(isize),
    Anonymous,
}
arena_deserializer::impl_deserialize_in_arena!(EmitId);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Class_<Ex, Fb, En> {
    pub span: Pos,
    pub annotation: En,
    pub mode: file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: ast_defs::ClassishKind,
    pub name: Sid,
    /// The type parameters of a class A<T> (T is the parameter)
    pub tparams: Vec<Tparam<Ex, Fb, En>>,
    pub extends: Vec<ClassHint>,
    pub uses: Vec<TraitHint>,
    /// PHP feature not supported in hack but required
    /// because we have runtime support.
    pub use_as_alias: Vec<UseAsAlias>,
    /// PHP feature not supported in hack but required
    /// because we have runtime support.
    pub insteadof_alias: Vec<InsteadofAlias>,
    pub xhp_attr_uses: Vec<XhpAttrHint>,
    pub xhp_category: Option<(Pos, Vec<Pstring>)>,
    pub reqs: Vec<(ClassHint, IsExtends)>,
    pub implements: Vec<ClassHint>,
    pub support_dynamic_type: bool,
    pub where_constraints: Vec<WhereConstraintHint>,
    pub consts: Vec<ClassConst<Ex, Fb, En>>,
    pub typeconsts: Vec<ClassTypeconstDef<Ex, Fb, En>>,
    pub vars: Vec<ClassVar<Ex, Fb, En>>,
    pub methods: Vec<Method_<Ex, Fb, En>>,
    pub attributes: Vec<ClassAttr<Ex, Fb, En>>,
    pub xhp_children: Vec<(Pos, XhpChild)>,
    pub xhp_attrs: Vec<XhpAttr<Ex, Fb, En>>,
    pub namespace: Nsenv,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    pub file_attributes: Vec<FileAttribute<Ex, Fb, En>>,
    pub enum_: Option<Enum_>,
    pub doc_comment: Option<DocComment>,
    pub emit_id: Option<EmitId>,
}

pub type ClassHint = Hint;

pub type TraitHint = Hint;

pub type XhpAttrHint = Hint;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum XhpAttrTag {
    Required,
    LateInit,
}
impl TrivialDrop for XhpAttrTag {}
arena_deserializer::impl_deserialize_in_arena!(XhpAttrTag);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct XhpAttr<Ex, Fb, En>(
    pub TypeHint<Ex>,
    pub ClassVar<Ex, Fb, En>,
    pub Option<XhpAttrTag>,
    pub Option<(Pos, Vec<Expr<Ex, Fb, En>>)>,
);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum ClassAttr<Ex, Fb, En> {
    CAName(Sid),
    CAField(CaField<Ex, Fb, En>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct CaField<Ex, Fb, En> {
    pub type_: CaType,
    pub id: Sid,
    pub value: Option<Expr<Ex, Fb, En>>,
    pub required: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum CaType {
    CAHint(Hint),
    CAEnum(Vec<String>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum ClassConstKind<Ex, Fb, En> {
    /// CCAbstract represents the states
    ///    abstract const int X;
    ///    abstract const int Y = 4;
    /// The expr option is a default value
    CCAbstract(Option<Expr<Ex, Fb, En>>),
    /// CCConcrete represents
    ///    const int Z = 4;
    /// The expr is the value of the constant. It is not optional
    CCConcrete(Expr<Ex, Fb, En>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ClassConst<Ex, Fb, En> {
    pub type_: Option<Hint>,
    pub id: Sid,
    pub kind: ClassConstKind<Ex, Fb, En>,
    pub doc_comment: Option<DocComment>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ClassAbstractTypeconst {
    pub as_constraint: Option<Hint>,
    pub super_constraint: Option<Hint>,
    pub default: Option<Hint>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ClassConcreteTypeconst {
    pub c_tc_type: Hint,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ClassPartiallyAbstractTypeconst {
    pub constraint: Hint,
    pub type_: Hint,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum ClassTypeconst {
    TCAbstract(ClassAbstractTypeconst),
    TCConcrete(ClassConcreteTypeconst),
    TCPartiallyAbstract(ClassPartiallyAbstractTypeconst),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ClassTypeconstDef<Ex, Fb, En> {
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    pub name: Sid,
    pub kind: ClassTypeconst,
    pub span: Pos,
    pub doc_comment: Option<DocComment>,
    pub is_ctx: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct XhpAttrInfo {
    pub tag: Option<XhpAttrTag>,
    pub enum_values: Vec<ast_defs::XhpEnumValue>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ClassVar<Ex, Fb, En> {
    pub final_: bool,
    pub xhp_attr: Option<XhpAttrInfo>,
    pub abstract_: bool,
    pub readonly: bool,
    pub visibility: Visibility,
    pub type_: TypeHint<Ex>,
    pub id: Sid,
    pub expr: Option<Expr<Ex, Fb, En>>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    pub doc_comment: Option<DocComment>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    pub span: Pos,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Method_<Ex, Fb, En> {
    pub span: Pos,
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub readonly_this: bool,
    pub visibility: Visibility,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En>>,
    pub where_constraints: Vec<WhereConstraintHint>,
    pub variadic: FunVariadicity<Ex, Fb, En>,
    pub params: Vec<FunParam<Ex, Fb, En>>,
    pub ctxs: Option<Contexts>,
    pub unsafe_ctxs: Option<Contexts>,
    pub body: FuncBody<Ex, Fb, En>,
    pub fun_kind: ast_defs::FunKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    pub readonly_ret: Option<ast_defs::ReadonlyKind>,
    pub ret: TypeHint<Ex>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<DocComment>,
}

pub type Nsenv = ocamlrep::rc::RcOc<namespace_env::Env>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Typedef<Ex, Fb, En> {
    pub annotation: En,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En>>,
    pub constraint: Option<Hint>,
    pub kind: Hint,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    pub mode: file_info::Mode,
    pub vis: TypedefVisibility,
    pub namespace: Nsenv,
    pub span: Pos,
    pub emit_id: Option<EmitId>,
    pub is_ctx: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Gconst<Ex, Fb, En> {
    pub annotation: En,
    pub mode: file_info::Mode,
    pub name: Sid,
    pub type_: Option<Hint>,
    pub value: Expr<Ex, Fb, En>,
    pub namespace: Nsenv,
    pub span: Pos,
    pub emit_id: Option<EmitId>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct RecordDef<Ex, Fb, En> {
    pub annotation: En,
    pub name: Sid,
    pub extends: Option<RecordHint>,
    pub abstract_: bool,
    pub fields: Vec<(Sid, Hint, Option<Expr<Ex, Fb, En>>)>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En>>,
    pub namespace: Nsenv,
    pub span: Pos,
    pub doc_comment: Option<DocComment>,
    pub emit_id: Option<EmitId>,
}

pub type RecordHint = Hint;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct FunDef<Ex, Fb, En> {
    pub namespace: Nsenv,
    pub file_attributes: Vec<FileAttribute<Ex, Fb, En>>,
    pub mode: file_info::Mode,
    pub fun: Fun_<Ex, Fb, En>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Def<Ex, Fb, En> {
    Fun(Box<FunDef<Ex, Fb, En>>),
    Class(Box<Class_<Ex, Fb, En>>),
    RecordDef(Box<RecordDef<Ex, Fb, En>>),
    Stmt(Box<Stmt<Ex, Fb, En>>),
    Typedef(Box<Typedef<Ex, Fb, En>>),
    Constant(Box<Gconst<Ex, Fb, En>>),
    Namespace(Box<(Sid, Program<Ex, Fb, En>)>),
    NamespaceUse(Vec<(NsKind, Sid, Sid)>),
    SetNamespaceEnv(Box<Nsenv>),
    FileAttributes(Box<FileAttribute<Ex, Fb, En>>),
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum NsKind {
    NSNamespace,
    NSClass,
    NSClassAndNamespace,
    NSFun,
    NSConst,
}
impl TrivialDrop for NsKind {}
arena_deserializer::impl_deserialize_in_arena!(NsKind);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum BreakContinueLevel {
    LevelOk(Option<isize>),
    LevelNonLiteral,
    LevelNonPositive,
}
