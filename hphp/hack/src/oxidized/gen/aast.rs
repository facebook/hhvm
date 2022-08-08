// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<b24cbf90e05132e16ec33bbae889dfaf>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use aast_defs::*;
use arena_trait::TrivialDrop;
pub use doc_comment::DocComment;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
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
#[repr(C)]
pub struct Program<Ex, En>(pub Vec<Def<Ex, En>>);

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
#[repr(C)]
pub struct Stmt<Ex, En>(pub Pos, pub Stmt_<Ex, En>);

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
#[repr(C, u8)]
pub enum Stmt_<Ex, En> {
    /// Marker for a switch statement that falls through.
    ///
    /// // FALLTHROUGH
    Fallthrough,
    /// Standalone expression.
    ///
    /// 1 + 2;
    Expr(Box<Expr<Ex, En>>),
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
    Throw(Box<Expr<Ex, En>>),
    /// Return, with an optional value.
    ///
    /// return;
    /// return $foo;
    Return(Box<Option<Expr<Ex, En>>>),
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
    Awaitall(Box<(Vec<(Option<Lid>, Expr<Ex, En>)>, Block<Ex, En>)>),
    /// If statement.
    ///
    /// if ($foo) { ... } else { ... }
    If(Box<(Expr<Ex, En>, Block<Ex, En>, Block<Ex, En>)>),
    /// Do-while loop.
    ///
    /// do {
    /// bar();
    /// } while($foo)
    Do(Box<(Block<Ex, En>, Expr<Ex, En>)>),
    /// While loop.
    ///
    /// while ($foo) {
    /// bar();
    /// }
    While(Box<(Expr<Ex, En>, Block<Ex, En>)>),
    /// Initialize a value that is automatically disposed of.
    ///
    /// using $foo = bar(); // disposed at the end of the function
    /// using ($foo = bar(), $baz = quux()) {} // disposed after the block
    Using(Box<UsingStmt<Ex, En>>),
    /// For loop. The initializer and increment parts can include
    /// multiple comma-separated statements. The termination condition is
    /// optional.
    ///
    /// for ($i = 0; $i < 100; $i++) { ... }
    /// for ($x = 0, $y = 0; ; $x++, $y++) { ... }
    For(
        Box<(
            Vec<Expr<Ex, En>>,
            Option<Expr<Ex, En>>,
            Vec<Expr<Ex, En>>,
            Block<Ex, En>,
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
    Switch(Box<(Expr<Ex, En>, Vec<Case<Ex, En>>, Option<DefaultCase<Ex, En>>)>),
    /// For-each loop.
    ///
    /// foreach ($items as $item) { ... }
    /// foreach ($items as $key => value) { ... }
    /// foreach ($items await as $item) { ... } // AsyncIterator<_>
    /// foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_>
    Foreach(Box<(Expr<Ex, En>, AsExpr<Ex, En>, Block<Ex, En>)>),
    /// Try statement, with catch blocks and a finally block.
    ///
    /// try {
    /// foo();
    /// } catch (SomeException $e) {
    /// bar();
    /// } finally {
    /// baz();
    /// }
    Try(Box<(Block<Ex, En>, Vec<Catch<Ex, En>>, Block<Ex, En>)>),
    /// No-op, the empty statement.
    ///
    /// {}
    /// while (true) ;
    /// if ($foo) {} // the else is Noop here
    Noop,
    /// Block, a list of statements in curly braces.
    ///
    /// { $foo = 42; }
    Block(Block<Ex, En>),
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
#[repr(u8)]
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
#[rust_to_ocaml(prefix = "us_")]
#[repr(C)]
pub struct UsingStmt<Ex, En> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    pub exprs: (Pos, Vec<Expr<Ex, En>>),
    pub block: Block<Ex, En>,
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
#[repr(C, u8)]
pub enum AsExpr<Ex, En> {
    AsV(Expr<Ex, En>),
    AsKv(Expr<Ex, En>, Expr<Ex, En>),
    AwaitAsV(Pos, Expr<Ex, En>),
    AwaitAsKv(Pos, Expr<Ex, En>, Expr<Ex, En>),
}

pub type Block<Ex, En> = Vec<Stmt<Ex, En>>;

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
#[repr(C)]
pub struct ClassId<Ex, En>(pub Ex, pub Pos, pub ClassId_<Ex, En>);

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
#[repr(C, u8)]
pub enum ClassId_<Ex, En> {
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
    CIexpr(Expr<Ex, En>),
    /// Explicit class name. This is the common case.
    ///
    /// Foop::some_meth()
    /// Foo::$prop = 1;
    /// new Foo();
    CI(ClassName),
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
#[repr(C)]
pub struct Expr<Ex, En>(pub Ex, pub Pos, pub Expr_<Ex, En>);

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
#[repr(C, u8)]
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
#[repr(C, u8)]
pub enum FunctionPtrId<Ex, En> {
    FPId(Sid),
    FPClassConst(ClassId<Ex, En>, Pstring),
}

/// An expression tree literal consists of a hint, splices, and
/// expressions. Consider this example:
///
/// Foo`1 + ${$x} + ${bar()}`
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
#[rust_to_ocaml(prefix = "et_")]
#[repr(C)]
pub struct ExpressionTree<Ex, En> {
    /// The hint before the backtick, so Foo in this example.
    pub hint: Hint,
    /// The values spliced into expression tree at runtime are assigned
    /// to temporaries.
    ///
    /// $0tmp1 = $x; $0tmp2 = bar();
    pub splices: Vec<Stmt<Ex, En>>,
    /// The list of global functions and static methods assigned to
    /// temporaries.
    ///
    /// $0fp1 = foo<>;
    pub function_pointers: Vec<Stmt<Ex, En>>,
    /// The expression that gets type checked.
    ///
    /// 1 + $0tmp1 + $0tmp2
    pub virtualized_expr: Expr<Ex, En>,
    /// The expression that's executed at runtime.
    ///
    /// Foo::makeTree($v ==> $v->visitBinOp(...))
    pub runtime_expr: Expr<Ex, En>,
    /// Position of the first $$ in a splice that refers
    /// to a variable outside the Expression Tree
    ///
    /// $x |> Code`${ $$ }` // Pos of the $$
    /// Code`${ $x |> foo($$) }` // None
    pub dollardollar_pos: Option<Pos>,
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
#[repr(C, u8)]
pub enum Expr_<Ex, En> {
    /// darray literal.
    ///
    /// darray['x' => 0, 'y' => 1]
    /// darray<string, int>['x' => 0, 'y' => 1]
    Darray(
        Box<(
            Option<(Targ<Ex>, Targ<Ex>)>,
            Vec<(Expr<Ex, En>, Expr<Ex, En>)>,
        )>,
    ),
    /// varray literal.
    ///
    /// varray['hello', 'world']
    /// varray<string>['hello', 'world']
    Varray(Box<(Option<Targ<Ex>>, Vec<Expr<Ex, En>>)>),
    /// Shape literal.
    ///
    /// shape('x' => 1, 'y' => 2)
    Shape(Vec<(ast_defs::ShapeFieldName, Expr<Ex, En>)>),
    /// Collection literal for indexable structures.
    ///
    /// Vector {1, 2}
    /// ImmVector {}
    /// Set<string> {'foo', 'bar'}
    /// vec[1, 2]
    /// keyset[]
    ValCollection(Box<(VcKind, Option<Targ<Ex>>, Vec<Expr<Ex, En>>)>),
    /// Collection literal for key-value structures.
    ///
    /// dict['x' => 1, 'y' => 2]
    /// Map<int, string> {}
    /// ImmMap {}
    KeyValCollection(Box<(KvcKind, Option<(Targ<Ex>, Targ<Ex>)>, Vec<Field<Ex, En>>)>),
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
    Clone(Box<Expr<Ex, En>>),
    /// Array indexing.
    ///
    /// $foo[]
    /// $foo[$bar]
    ArrayGet(Box<(Expr<Ex, En>, Option<Expr<Ex, En>>)>),
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
    ObjGet(Box<(Expr<Ex, En>, Expr<Ex, En>, OgNullFlavor, PropOrMethod)>),
    /// Static property or method access.
    ///
    /// Foo::$bar               // Is_prop
    /// $some_classname::$bar   // Is_prop
    /// Foo::${$bar}            // Is_prop, only in partial mode
    ///
    /// Foo::bar();             // Is_method
    /// Foo::$bar();            // Is_method, name stored in local $bar
    /// (Foo::$bar)();          // Is_prop: call lambda stored in property Foo::$bar
    ClassGet(Box<(ClassId<Ex, En>, ClassGetExpr<Ex, En>, PropOrMethod)>),
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
    ClassConst(Box<(ClassId<Ex, En>, Pstring)>),
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
    Call(
        Box<(
            Expr<Ex, En>,
            Vec<Targ<Ex>>,
            Vec<(ast_defs::ParamKind, Expr<Ex, En>)>,
            Option<Expr<Ex, En>>,
        )>,
    ),
    /// A reference to a function or method.
    ///
    /// foo_fun<>
    /// FooCls::meth<int>
    FunctionPointer(Box<(FunctionPtrId<Ex, En>, Vec<Targ<Ex>>)>),
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
    String2(Vec<Expr<Ex, En>>),
    /// Prefixed string literal. Only used for regular expressions.
    ///
    /// re"foo"
    PrefixedString(Box<(String, Expr<Ex, En>)>),
    /// Yield expression. The enclosing function should have an Iterator
    /// return type.
    ///
    /// yield $foo // enclosing function returns an Iterator
    /// yield $foo => $bar // enclosing function returns a KeyedIterator
    Yield(Box<Afield<Ex, En>>),
    /// Await expression.
    ///
    /// await $foo
    Await(Box<Expr<Ex, En>>),
    /// Readonly expression.
    ///
    /// readonly $foo
    ReadonlyExpr(Box<Expr<Ex, En>>),
    /// Tuple expression.
    ///
    /// tuple("a", 1, $foo)
    Tuple(Vec<Expr<Ex, En>>),
    /// List expression, only used in destructuring. Allows any arbitrary
    /// lvalue as a subexpression. May also nest.
    ///
    /// list($x, $y) = vec[1, 2];
    /// list(, $y) = vec[1, 2]; // skipping items
    /// list(list($x)) = vec[vec[1]]; // nesting
    /// list($v[0], $x[], $y->foo) = $blah;
    List(Vec<Expr<Ex, En>>),
    /// Cast expression, converting a value to a different type. Only
    /// primitive types are supported in the hint position.
    ///
    /// (int)$foo
    /// (string)$foo
    Cast(Box<(Hint, Expr<Ex, En>)>),
    /// Unary operator.
    ///
    /// !$foo
    /// -$foo
    /// +$foo
    /// $foo++
    Unop(Box<(ast_defs::Uop, Expr<Ex, En>)>),
    /// Binary operator.
    ///
    /// $foo + $bar
    Binop(Box<(ast_defs::Bop, Expr<Ex, En>, Expr<Ex, En>)>),
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
    Pipe(Box<(Lid, Expr<Ex, En>, Expr<Ex, En>)>),
    /// Ternary operator, or elvis operator.
    ///
    /// $foo ? $bar : $baz // ternary
    /// $foo ?: $baz // elvis
    Eif(Box<(Expr<Ex, En>, Option<Expr<Ex, En>>, Expr<Ex, En>)>),
    /// Is operator.
    ///
    /// $foo is SomeType
    Is(Box<(Expr<Ex, En>, Hint)>),
    /// As operator.
    ///
    /// $foo as int
    /// $foo ?as int
    As(Box<(Expr<Ex, En>, Hint, bool)>),
    /// Upcast operator.
    ///
    /// $foo : int
    Upcast(Box<(Expr<Ex, En>, Hint)>),
    /// Instantiation.
    ///
    /// new Foo(1, 2);
    /// new Foo<int, T>();
    /// new Foo('blah', ...$rest);
    New(
        Box<(
            ClassId<Ex, En>,
            Vec<Targ<Ex>>,
            Vec<Expr<Ex, En>>,
            Option<Expr<Ex, En>>,
            Ex,
        )>,
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
    Efun(Box<(Fun_<Ex, En>, Vec<Lid>)>),
    /// Hack lambda. Captures variables automatically.
    ///
    /// $x ==> $x
    /// (int $x): int ==> $x + $other
    /// ($x, $y) ==> { return $x + $y; }
    Lfun(Box<(Fun_<Ex, En>, Vec<Lid>)>),
    /// XHP expression. May contain interpolated expressions.
    ///
    /// <foo x="hello" y={$foo}>hello {$bar}</foo>
    Xml(Box<(ClassName, Vec<XhpAttribute<Ex, En>>, Vec<Expr<Ex, En>>)>),
    /// Include or require expression.
    ///
    /// require('foo.php')
    /// require_once('foo.php')
    /// include('foo.php')
    /// include_once('foo.php')
    Import(Box<(ImportFlavor, Expr<Ex, En>)>),
    /// Collection literal.
    ///
    /// TODO: T38184446 this is redundant with ValCollection/KeyValCollection.
    ///
    /// Vector {}
    Collection(Box<(ClassName, Option<CollectionTarg<Ex>>, Vec<Afield<Ex, En>>)>),
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
    ExpressionTree(Box<ExpressionTree<Ex, En>>),
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
    MethodId(Box<(Expr<Ex, En>, Pstring)>),
    /// Instance method reference that can be called with an instance.
    ///
    /// meth_caller(FooClass::class, 'some_meth')
    /// meth_caller('FooClass', 'some_meth')
    ///
    /// These examples are equivalent to:
    ///
    /// (FooClass $f, ...$args) ==> $f->some_meth(...$args)
    MethodCaller(Box<(ClassName, Pstring)>),
    /// Static method reference.
    ///
    /// class_meth('FooClass', 'some_static_meth')
    /// // equivalent: FooClass::some_static_meth<>
    SmethodId(Box<(ClassId<Ex, En>, Pstring)>),
    /// Pair literal.
    ///
    /// Pair {$foo, $bar}
    Pair(Box<(Option<(Targ<Ex>, Targ<Ex>)>, Expr<Ex, En>, Expr<Ex, En>)>),
    /// Expression tree splice expression. Only valid inside an
    /// expression tree literal (backticks). See also `ExpressionTree`.
    ///
    /// ${$foo}
    ETSplice(Box<Expr<Ex, En>>),
    /// Label used for enum classes.
    ///
    /// enum_name#label_name or #label_name
    EnumClassLabel(Box<(Option<ClassName>, String)>),
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
    Hole(Box<(Expr<Ex, En>, Ex, Ex, HoleSource)>),
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
#[repr(C, u8)]
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
#[repr(C, u8)]
pub enum ClassGetExpr<Ex, En> {
    CGstring(Pstring),
    CGexpr(Expr<Ex, En>),
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
#[repr(C)]
pub struct Case<Ex, En>(pub Expr<Ex, En>, pub Block<Ex, En>);

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
#[repr(C)]
pub struct DefaultCase<Ex, En>(pub Pos, pub Block<Ex, En>);

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
#[repr(C, u8)]
pub enum GenCase<Ex, En> {
    Case(Case<Ex, En>),
    Default(DefaultCase<Ex, En>),
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
#[repr(C)]
pub struct Catch<Ex, En>(pub ClassName, pub Lid, pub Block<Ex, En>);

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
#[repr(C)]
pub struct Field<Ex, En>(pub Expr<Ex, En>, pub Expr<Ex, En>);

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
#[repr(C, u8)]
pub enum Afield<Ex, En> {
    AFvalue(Expr<Ex, En>),
    AFkvalue(Expr<Ex, En>, Expr<Ex, En>),
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
#[rust_to_ocaml(prefix = "xs_")]
#[repr(C)]
pub struct XhpSimple<Ex, En> {
    pub name: Pstring,
    pub type_: Ex,
    pub expr: Expr<Ex, En>,
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
#[repr(C, u8)]
pub enum XhpAttribute<Ex, En> {
    XhpSimple(XhpSimple<Ex, En>),
    XhpSpread(Expr<Ex, En>),
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
#[rust_to_ocaml(prefix = "param_")]
#[repr(C)]
pub struct FunParam<Ex, En> {
    pub annotation: Ex,
    pub type_hint: TypeHint<Ex>,
    pub is_variadic: IsVariadic,
    pub pos: Pos,
    pub name: String,
    pub expr: Option<Expr<Ex, En>>,
    pub readonly: Option<ast_defs::ReadonlyKind>,
    pub callconv: ast_defs::ParamKind,
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
    pub visibility: Option<Visibility>,
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
#[rust_to_ocaml(prefix = "f_")]
#[repr(C)]
pub struct Fun_<Ex, En> {
    pub span: Pos,
    pub readonly_this: Option<ast_defs::ReadonlyKind>,
    pub annotation: En,
    pub readonly_ret: Option<ast_defs::ReadonlyKind>,
    pub ret: TypeHint<Ex>,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, En>>,
    pub where_constraints: Vec<WhereConstraintHint>,
    pub params: Vec<FunParam<Ex, En>>,
    pub ctxs: Option<Contexts>,
    pub unsafe_ctxs: Option<Contexts>,
    pub body: FuncBody<Ex, En>,
    pub fun_kind: ast_defs::FunKind,
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<DocComment>,
}

/// Naming has two phases and the annotation helps to indicate the phase.
/// In the first pass, it will perform naming on everything except for function
/// and method bodies and collect information needed. Then, another round of
/// naming is performed where function bodies are named.
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
#[repr(C)]
pub struct FuncBody<Ex, En> {
    pub fb_ast: Block<Ex, En>,
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
#[repr(C)]
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
#[repr(C)]
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
#[rust_to_ocaml(prefix = "ua_")]
#[repr(C)]
pub struct UserAttribute<Ex, En> {
    pub name: Sid,
    /// user attributes are restricted to scalar values
    pub params: Vec<Expr<Ex, En>>,
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
#[rust_to_ocaml(prefix = "fa_")]
#[repr(C)]
pub struct FileAttribute<Ex, En> {
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
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
#[rust_to_ocaml(prefix = "tp_")]
#[repr(C)]
pub struct Tparam<Ex, En> {
    pub variance: ast_defs::Variance,
    pub name: Sid,
    pub parameters: Vec<Tparam<Ex, En>>,
    pub constraints: Vec<(ast_defs::ConstraintKind, Hint)>,
    pub reified: ReifyKind,
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
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
#[repr(u8)]
pub enum RequireKind {
    RequireExtends,
    RequireImplements,
    RequireClass,
}
impl TrivialDrop for RequireKind {}
arena_deserializer::impl_deserialize_in_arena!(RequireKind);

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
#[repr(C, u8)]
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
#[rust_to_ocaml(prefix = "c_")]
#[repr(C)]
pub struct Class_<Ex, En> {
    pub span: Pos,
    pub annotation: En,
    pub mode: file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: ast_defs::ClassishKind,
    pub name: ClassName,
    /// The type parameters of a class A<T> (T is the parameter)
    pub tparams: Vec<Tparam<Ex, En>>,
    pub extends: Vec<ClassHint>,
    pub uses: Vec<TraitHint>,
    pub xhp_attr_uses: Vec<XhpAttrHint>,
    pub xhp_category: Option<(Pos, Vec<Pstring>)>,
    pub reqs: Vec<(ClassHint, RequireKind)>,
    pub implements: Vec<ClassHint>,
    pub where_constraints: Vec<WhereConstraintHint>,
    pub consts: Vec<ClassConst<Ex, En>>,
    pub typeconsts: Vec<ClassTypeconstDef<Ex, En>>,
    pub vars: Vec<ClassVar<Ex, En>>,
    pub methods: Vec<Method_<Ex, En>>,
    pub xhp_children: Vec<(Pos, XhpChild)>,
    pub xhp_attrs: Vec<XhpAttr<Ex, En>>,
    pub namespace: Nsenv,
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
    pub file_attributes: Vec<FileAttribute<Ex, En>>,
    pub docs_url: Option<String>,
    pub enum_: Option<Enum_>,
    pub doc_comment: Option<DocComment>,
    pub emit_id: Option<EmitId>,
    pub internal: bool,
    pub module: Option<Sid>,
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
#[repr(u8)]
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
#[repr(C)]
pub struct XhpAttr<Ex, En>(
    pub TypeHint<Ex>,
    pub ClassVar<Ex, En>,
    pub Option<XhpAttrTag>,
    pub Option<(Pos, Vec<Expr<Ex, En>>)>,
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
#[repr(C, u8)]
pub enum ClassConstKind<Ex, En> {
    /// CCAbstract represents the states
    ///    abstract const int X;
    ///    abstract const int Y = 4;
    /// The expr option is a default value
    CCAbstract(Option<Expr<Ex, En>>),
    /// CCConcrete represents
    ///    const int Z = 4;
    /// The expr is the value of the constant. It is not optional
    CCConcrete(Expr<Ex, En>),
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
#[rust_to_ocaml(prefix = "cc_")]
#[repr(C)]
pub struct ClassConst<Ex, En> {
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
    pub type_: Option<Hint>,
    pub id: Sid,
    pub kind: ClassConstKind<Ex, En>,
    pub span: Pos,
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
#[rust_to_ocaml(prefix = "c_atc_")]
#[repr(C)]
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
#[repr(C)]
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
#[repr(C, u8)]
pub enum ClassTypeconst {
    TCAbstract(ClassAbstractTypeconst),
    TCConcrete(ClassConcreteTypeconst),
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
#[rust_to_ocaml(prefix = "c_tconst_")]
#[repr(C)]
pub struct ClassTypeconstDef<Ex, En> {
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
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
#[rust_to_ocaml(prefix = "xai_")]
#[repr(C)]
pub struct XhpAttrInfo {
    pub like: Option<Pos>,
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
#[rust_to_ocaml(prefix = "cv_")]
#[repr(C)]
pub struct ClassVar<Ex, En> {
    pub final_: bool,
    pub xhp_attr: Option<XhpAttrInfo>,
    pub abstract_: bool,
    pub readonly: bool,
    pub visibility: Visibility,
    pub type_: TypeHint<Ex>,
    pub id: Sid,
    pub expr: Option<Expr<Ex, En>>,
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
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
#[rust_to_ocaml(prefix = "m_")]
#[repr(C)]
pub struct Method_<Ex, En> {
    pub span: Pos,
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub readonly_this: bool,
    pub visibility: Visibility,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, En>>,
    pub where_constraints: Vec<WhereConstraintHint>,
    pub params: Vec<FunParam<Ex, En>>,
    pub ctxs: Option<Contexts>,
    pub unsafe_ctxs: Option<Contexts>,
    pub body: FuncBody<Ex, En>,
    pub fun_kind: ast_defs::FunKind,
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
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
#[rust_to_ocaml(prefix = "t_")]
#[repr(C)]
pub struct Typedef<Ex, En> {
    pub annotation: En,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, En>>,
    pub constraint: Option<Hint>,
    pub kind: Hint,
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
    pub file_attributes: Vec<FileAttribute<Ex, En>>,
    pub mode: file_info::Mode,
    pub vis: TypedefVisibility,
    pub namespace: Nsenv,
    pub span: Pos,
    pub emit_id: Option<EmitId>,
    pub is_ctx: bool,
    pub internal: bool,
    pub module: Option<Sid>,
    pub docs_url: Option<String>,
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
#[rust_to_ocaml(prefix = "cst_")]
#[repr(C)]
pub struct Gconst<Ex, En> {
    pub annotation: En,
    pub mode: file_info::Mode,
    pub name: Sid,
    pub type_: Option<Hint>,
    pub value: Expr<Ex, En>,
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
#[rust_to_ocaml(prefix = "fd_")]
#[repr(C)]
pub struct FunDef<Ex, En> {
    pub namespace: Nsenv,
    pub file_attributes: Vec<FileAttribute<Ex, En>>,
    pub mode: file_info::Mode,
    pub fun: Fun_<Ex, En>,
    pub internal: bool,
    pub module: Option<Sid>,
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
#[rust_to_ocaml(prefix = "md_")]
#[repr(C)]
pub struct ModuleDef<Ex, En> {
    pub annotation: En,
    pub name: ast_defs::Id,
    pub user_attributes: Vec<UserAttribute<Ex, En>>,
    pub span: Pos,
    pub mode: file_info::Mode,
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
#[repr(C, u8)]
pub enum Def<Ex, En> {
    Fun(Box<FunDef<Ex, En>>),
    Class(Box<Class_<Ex, En>>),
    Stmt(Box<Stmt<Ex, En>>),
    Typedef(Box<Typedef<Ex, En>>),
    Constant(Box<Gconst<Ex, En>>),
    Namespace(Box<(Sid, Vec<Def<Ex, En>>)>),
    NamespaceUse(Vec<(NsKind, Sid, Sid)>),
    SetNamespaceEnv(Box<Nsenv>),
    FileAttributes(Box<FileAttribute<Ex, En>>),
    Module(Box<ModuleDef<Ex, En>>),
    SetModule(Box<Sid>),
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
#[repr(u8)]
pub enum NsKind {
    NSNamespace,
    NSClass,
    NSClassAndNamespace,
    NSFun,
    NSConst,
}
impl TrivialDrop for NsKind {}
arena_deserializer::impl_deserialize_in_arena!(NsKind);
