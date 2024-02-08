// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6e13f5a2e5bb96e2d86f8b2ff4b7f1e9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
pub use ast_defs::OgNullFlavor;
pub use ast_defs::Pos;
pub use ast_defs::PositionedByteString;
pub use ast_defs::PropOrMethod;
pub use ast_defs::Pstring;
pub use ast_defs::ReifyKind;
pub use ast_defs::Tprim;
pub use ast_defs::TypedefVisibility;
pub use ast_defs::Visibility;
pub use local_id::LocalId;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C)]
pub struct Lid(pub Pos, pub LocalId);

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
pub type Sid = ast_defs::Id;

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
pub type ClassName = Sid;

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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Stmt_<Ex, En> {
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
    Expr(Box<Expr<Ex, En>>),
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
    Throw(Box<Expr<Ex, En>>),
    /// Return, with an optional value.
    ///
    ///     return;
    ///     return $foo;
    Return(Box<Option<Expr<Ex, En>>>),
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
    #[rust_to_ocaml(inline_tuple)]
    Awaitall(Box<(Vec<(Lid, Expr<Ex, En>)>, Block<Ex, En>)>),
    /// Concurrent block. All the await expressions are awaited at the
    /// same time, similar to genva().
    ///
    ///     concurrent {
    ///       $foo = await f();
    ///       $bar = await g();
    ///       await h();
    ///     }
    Concurrent(Block<Ex, En>),
    /// If statement.
    ///
    ///     if ($foo) { ... } else { ... }
    #[rust_to_ocaml(inline_tuple)]
    If(Box<(Expr<Ex, En>, Block<Ex, En>, Block<Ex, En>)>),
    /// Do-while loop.
    ///
    ///     do {
    ///       bar();
    ///     } while($foo)
    #[rust_to_ocaml(inline_tuple)]
    Do(Box<(Block<Ex, En>, Expr<Ex, En>)>),
    /// While loop.
    ///
    ///     while ($foo) {
    ///       bar();
    ///     }
    #[rust_to_ocaml(inline_tuple)]
    While(Box<(Expr<Ex, En>, Block<Ex, En>)>),
    /// Initialize a value that is automatically disposed of.
    ///
    ///     using $foo = bar(); // disposed at the end of the function
    ///     using ($foo = bar(), $baz = quux()) {} // disposed after the block
    Using(Box<UsingStmt<Ex, En>>),
    /// For loop. The initializer and increment parts can include
    /// multiple comma-separated statements. The termination condition is
    /// optional.
    ///
    ///     for ($i = 0; $i < 100; $i++) { ... }
    ///     for ($x = 0, $y = 0; ; $x++, $y++) { ... }
    #[rust_to_ocaml(inline_tuple)]
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
    ///     switch ($foo) {
    ///       case X:
    ///         bar();
    ///         break;
    ///       default:
    ///         baz();
    ///         break;
    ///     }
    #[rust_to_ocaml(inline_tuple)]
    Switch(Box<(Expr<Ex, En>, Vec<Case<Ex, En>>, Option<DefaultCase<Ex, En>>)>),
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
    Match(Box<StmtMatch<Ex, En>>),
    /// For-each loop.
    ///
    ///     foreach ($items as $item) { ... }
    ///     foreach ($items as $key => value) { ... }
    ///     foreach ($items await as $item) { ... } // AsyncIterator<_>
    ///     foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_>
    #[rust_to_ocaml(inline_tuple)]
    Foreach(Box<(Expr<Ex, En>, AsExpr<Ex, En>, Block<Ex, En>)>),
    /// Try statement, with catch blocks and a finally block.
    ///
    ///     try {
    ///       foo();
    ///     } catch (SomeException $e) {
    ///       bar();
    ///     } finally {
    ///       baz();
    ///     }
    #[rust_to_ocaml(inline_tuple)]
    Try(Box<(Block<Ex, En>, Vec<Catch<Ex, En>>, FinallyBlock<Ex, En>)>),
    /// Declare a local variable with the given type and optional initial value
    #[rust_to_ocaml(name = "Declare_local")]
    #[rust_to_ocaml(inline_tuple)]
    DeclareLocal(Box<(Lid, Hint, Option<Expr<Ex, En>>)>),
    /// Block, a list of statements in curly braces.
    ///
    ///     { $foo = 42; }
    /// If present, the optional list of identifiers are those that are scoped to this
    /// Block, they will be unset upon exit to the block.
    #[rust_to_ocaml(inline_tuple)]
    Block(Box<(Option<Vec<Lid>>, Block<Ex, En>)>),
    /// The mode tag at the beginning of a file.
    /// TODO: this really belongs in def.
    ///
    ///     <?hh
    #[rust_to_ocaml(attr = "transform.opaque")]
    Markup(Box<Pstring>),
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum AsExpr<Ex, En> {
    #[rust_to_ocaml(name = "As_v")]
    AsV(Expr<Ex, En>),
    #[rust_to_ocaml(name = "As_kv")]
    AsKv(Expr<Ex, En>, Expr<Ex, En>),
    #[rust_to_ocaml(name = "Await_as_v")]
    AwaitAsV(Pos, Expr<Ex, En>),
    #[rust_to_ocaml(name = "Await_as_kv")]
    AwaitAsKv(Pos, Expr<Ex, En>, Expr<Ex, En>),
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Block<Ex, En>(pub Vec<Stmt<Ex, En>>);

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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct FinallyBlock<Ex, En>(pub Vec<Stmt<Ex, En>>);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "sm_")]
#[repr(C)]
pub struct StmtMatch<Ex, En> {
    pub expr: Expr<Ex, En>,
    pub arms: Vec<StmtMatchArm<Ex, En>>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "sma_")]
#[repr(C)]
pub struct StmtMatchArm<Ex, En> {
    pub pat: Pattern,
    pub body: Block<Ex, En>,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Pattern {
    /// Variable patterns
    PVar(Box<PatVar>),
    /// Refinement patterns
    PRefinement(Box<PatRefinement>),
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "pv_")]
#[repr(C)]
pub struct PatVar {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub pos: Pos,
    pub id: Option<Lid>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "pr_")]
#[repr(C)]
pub struct PatRefinement {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub pos: Pos,
    pub id: Option<Lid>,
    pub hint: Hint,
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ClassId_<Ex, En> {
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
    CIexpr(Expr<Ex, En>),
    /// Explicit class name. This is the common case.
    ///
    ///     Foo::some_meth()
    ///     Foo::$prop = 1;
    ///     new Foo();
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum FunctionPtrId<Ex, En> {
    #[rust_to_ocaml(name = "FP_id")]
    FPId(Sid),
    /// An expression tree literal consists of a hint, splices, and
    ///  expressions. Consider this example:
    ///
    ///     Foo`1 + ${$x} + ${bar()}`
    #[rust_to_ocaml(name = "FP_class_const")]
    FPClassConst(ClassId<Ex, En>, Pstring),
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "et_")]
#[repr(C)]
pub struct ExpressionTree<Ex, En> {
    /// The hint before the backtick, so Foo in this example.
    pub class: ClassName,
    /// The values spliced into expression tree at runtime are assigned
    /// to temporaries.
    ///
    ///     $0tmp1 = $x; $0tmp2 = bar();
    pub splices: Vec<Stmt<Ex, En>>,
    /// The list of global functions and static methods assigned to
    /// temporaries.
    ///
    ///     $0fp1 = foo<>;
    pub function_pointers: Vec<Stmt<Ex, En>>,
    /// The expression that's executed at runtime.
    ///
    ///     Foo::makeTree($v ==> $v->visitBinOp(...))
    pub runtime_expr: Expr<Ex, En>,
    /// Position of the first $$ in a splice that refers
    /// to a variable outside the Expression Tree
    ///
    ///     $x |> Code`${ $$ }` // Pos of the $$
    ///     Code`${ $x |> foo($$) }` // None
    #[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct As_<Ex, En> {
    pub expr: Expr<Ex, En>,
    pub hint: Hint,
    pub is_nullable: bool,
    pub enforce_deep: bool,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Expr_<Ex, En> {
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
    Shape(Vec<(ast_defs::ShapeFieldName, Expr<Ex, En>)>),
    /// Collection literal for indexable structures.
    ///
    ///     Vector {1, 2}
    ///     ImmVector {}
    ///     Set<string> {'foo', 'bar'}
    ///     vec[1, 2]
    ///     keyset[]
    #[rust_to_ocaml(inline_tuple)]
    ValCollection(Box<((Pos, VcKind), Option<Targ<Ex>>, Vec<Expr<Ex, En>>)>),
    /// Collection literal for key-value structures.
    ///
    ///     dict['x' => 1, 'y' => 2]
    ///     Map<int, string> {}
    ///     ImmMap {}
    #[rust_to_ocaml(inline_tuple)]
    KeyValCollection(
        Box<(
            (Pos, KvcKind),
            Option<(Targ<Ex>, Targ<Ex>)>,
            Vec<Field<Ex, En>>,
        )>,
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
    Invalid(Box<Option<Expr<Ex, En>>>),
    /// An identifier. Used for method names and global constants.
    ///
    ///     SOME_CONST
    ///     $x->foo() // id: "foo"
    Id(Box<Sid>),
    /// Local variable.
    ///
    ///     $foo
    Lvar(Box<Lid>),
    /// The extra variable in a pipe expression.
    ///
    ///     $$
    Dollardollar(Box<Lid>),
    /// Clone expression.
    ///
    ///     clone $foo
    Clone(Box<Expr<Ex, En>>),
    /// Array indexing.
    ///
    ///     $foo[]
    ///     $foo[$bar]
    #[rust_to_ocaml(name = "Array_get")]
    #[rust_to_ocaml(inline_tuple)]
    ArrayGet(Box<(Expr<Ex, En>, Option<Expr<Ex, En>>)>),
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
    #[rust_to_ocaml(name = "Obj_get")]
    #[rust_to_ocaml(inline_tuple)]
    ObjGet(Box<(Expr<Ex, En>, Expr<Ex, En>, OgNullFlavor, PropOrMethod)>),
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
    #[rust_to_ocaml(name = "Class_get")]
    #[rust_to_ocaml(inline_tuple)]
    ClassGet(Box<(ClassId<Ex, En>, ClassGetExpr<Ex, En>, PropOrMethod)>),
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
    #[rust_to_ocaml(name = "Class_const")]
    #[rust_to_ocaml(inline_tuple)]
    ClassConst(Box<(ClassId<Ex, En>, Pstring)>),
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
    Call(Box<CallExpr<Ex, En>>),
    /// A reference to a function or method.
    ///
    ///     foo_fun<>
    ///     FooCls::meth<int>
    #[rust_to_ocaml(inline_tuple)]
    FunctionPointer(Box<(FunctionPtrId<Ex, En>, Vec<Targ<Ex>>)>),
    /// Integer literal.
    ///
    ///     42
    ///     0123 // octal
    ///     0xBEEF // hexadecimal
    ///     0b11111111 // binary
    Int(String),
    /// Float literal.
    ///
    ///     1.0
    ///     1.2e3
    ///     7E-10
    Float(String),
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
    #[rust_to_ocaml(attr = "transform.opaque")]
    String(bstr::BString),
    /// Interpolated string literal.
    ///
    ///     "hello $foo $bar"
    ///
    ///     <<<DOC
    ///     hello $foo $bar
    ///     DOC
    String2(Vec<Expr<Ex, En>>),
    /// Prefixed string literal. Only used for regular expressions.
    ///
    ///     re"foo"
    #[rust_to_ocaml(inline_tuple)]
    PrefixedString(Box<(String, Expr<Ex, En>)>),
    /// Yield expression. The enclosing function should have an Iterator
    /// return type.
    ///
    ///     yield $foo // enclosing function returns an Iterator
    ///     yield $foo => $bar // enclosing function returns a KeyedIterator
    Yield(Box<Afield<Ex, En>>),
    /// Await expression.
    ///
    ///     await $foo
    Await(Box<Expr<Ex, En>>),
    /// Readonly expression.
    ///
    ///     readonly $foo
    ReadonlyExpr(Box<Expr<Ex, En>>),
    /// Tuple expression.
    ///
    ///     tuple("a", 1, $foo)
    Tuple(Vec<Expr<Ex, En>>),
    /// List expression, only used in destructuring. Allows any arbitrary
    /// lvalue as a subexpression. May also nest.
    ///
    ///     list($x, $y) = vec[1, 2];
    ///     list(, $y) = vec[1, 2]; // skipping items
    ///     list(list($x)) = vec[vec[1]]; // nesting
    ///     list($v[0], $x[], $y->foo) = $blah;
    List(Vec<Expr<Ex, En>>),
    /// Cast expression, converting a value to a different type. Only
    /// primitive types are supported in the hint position.
    ///
    ///     (int)$foo
    ///     (string)$foo
    #[rust_to_ocaml(inline_tuple)]
    Cast(Box<(Hint, Expr<Ex, En>)>),
    /// Unary operator.
    ///
    ///     !$foo
    ///     -$foo
    ///     +$foo
    ///     $foo++
    #[rust_to_ocaml(inline_tuple)]
    Unop(Box<(ast_defs::Uop, Expr<Ex, En>)>),
    /// Binary operator.
    ///
    ///     $foo + $bar
    Binop(Box<Binop<Ex, En>>),
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
    #[rust_to_ocaml(inline_tuple)]
    Pipe(Box<(Lid, Expr<Ex, En>, Expr<Ex, En>)>),
    /// Ternary operator, or elvis operator.
    ///
    ///     $foo ? $bar : $baz // ternary
    ///     $foo ?: $baz // elvis
    #[rust_to_ocaml(inline_tuple)]
    Eif(Box<(Expr<Ex, En>, Option<Expr<Ex, En>>, Expr<Ex, En>)>),
    /// Is operator.
    ///
    ///     $foo is SomeType
    #[rust_to_ocaml(inline_tuple)]
    Is(Box<(Expr<Ex, En>, Hint)>),
    /// As operator.
    ///
    ///     $foo as int
    ///     $foo ?as int
    As(Box<As_<Ex, En>>),
    /// Upcast operator.
    ///
    ///     $foo : int
    #[rust_to_ocaml(inline_tuple)]
    Upcast(Box<(Expr<Ex, En>, Hint)>),
    /// Instantiation.
    ///
    ///     new Foo(1, 2);
    ///     new Foo<int, T>();
    ///     new Foo('blah', ...$rest);
    #[rust_to_ocaml(inline_tuple)]
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
    ///     function($x) { return $x; }
    ///     function(int $x): int { return $x; }
    ///     function($x) use ($y) { return $y; }
    ///     function($x): int use ($y, $z) { return $x + $y + $z; }
    Efun(Box<Efun<Ex, En>>),
    /// Hack lambda. Captures variables automatically.
    ///
    ///     $x ==> $x
    ///     (int $x): int ==> $x + $other
    ///     ($x, $y) ==> { return $x + $y; }
    #[rust_to_ocaml(inline_tuple)]
    Lfun(Box<(Fun_<Ex, En>, Vec<CaptureLid<Ex>>)>),
    /// XHP expression. May contain interpolated expressions.
    ///
    ///     <foo x="hello" y={$foo}>hello {$bar}</foo>
    #[rust_to_ocaml(inline_tuple)]
    Xml(Box<(ClassName, Vec<XhpAttribute<Ex, En>>, Vec<Expr<Ex, En>>)>),
    /// Include or require expression.
    ///
    ///     require('foo.php')
    ///     require_once('foo.php')
    ///     include('foo.php')
    ///     include_once('foo.php')
    #[rust_to_ocaml(inline_tuple)]
    Import(Box<(ImportFlavor, Expr<Ex, En>)>),
    /// Collection literal.
    ///
    /// TODO: T38184446 this is redundant with ValCollection/KeyValCollection.
    ///
    ///     Vector {}
    #[rust_to_ocaml(inline_tuple)]
    Collection(Box<(ClassName, Option<CollectionTarg<Ex>>, Vec<Afield<Ex, En>>)>),
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
    ExpressionTree(Box<ExpressionTree<Ex, En>>),
    /// Placeholder local variable.
    ///
    ///     $_
    #[rust_to_ocaml(attr = "transform.opaque")]
    Lplaceholder(Box<Pos>),
    /// Instance method reference that can be called with an instance.
    ///
    ///     meth_caller(FooClass::class, 'some_meth')
    ///     meth_caller('FooClass', 'some_meth')
    ///
    /// These examples are equivalent to:
    ///
    ///     (FooClass $f, ...$args) ==> $f->some_meth(...$args)
    #[rust_to_ocaml(name = "Method_caller")]
    #[rust_to_ocaml(inline_tuple)]
    MethodCaller(Box<(ClassName, Pstring)>),
    /// Pair literal.
    ///
    ///     Pair {$foo, $bar}
    #[rust_to_ocaml(inline_tuple)]
    Pair(Box<(Option<(Targ<Ex>, Targ<Ex>)>, Expr<Ex, En>, Expr<Ex, En>)>),
    /// Expression tree splice expression. Only valid inside an
    /// expression tree literal (backticks). See also `ExpressionTree`.
    ///
    ///     ${$foo}
    #[rust_to_ocaml(name = "ET_Splice")]
    ETSplice(Box<Expr<Ex, En>>),
    /// Label used for enum classes.
    ///
    ///     enum_name#label_name or #label_name
    #[rust_to_ocaml(inline_tuple)]
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
    #[rust_to_ocaml(inline_tuple)]
    Hole(Box<(Expr<Ex, En>, Ex, Ex, HoleSource)>),
    /// Expression used to check whether a package exists.
    ///
    ///     package package-name
    Package(Box<Sid>),
    /// Get the name of a class
    Nameof(Box<ClassId<Ex, En>>),
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum HoleSource {
    Typing,
    UnsafeCast(Vec<Hint>),
    UnsafeNonnullCast,
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Binop<Ex, En> {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub bop: ast_defs::Bop,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub lhs: Expr<Ex, En>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub rhs: Expr<Ex, En>,
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "xs_")]
#[repr(C)]
pub struct XhpSimple<Ex, En> {
    #[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum XhpAttribute<Ex, En> {
    #[rust_to_ocaml(name = "Xhp_simple")]
    XhpSimple(XhpSimple<Ex, En>),
    #[rust_to_ocaml(name = "Xhp_spread")]
    XhpSpread(Expr<Ex, En>),
}

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "param_")]
#[repr(C)]
pub struct FunParam<Ex, En> {
    pub annotation: Ex,
    pub type_hint: TypeHint<Ex>,
    pub is_variadic: IsVariadic,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub pos: Pos,
    pub name: String,
    pub expr: Option<Expr<Ex, En>>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub readonly: Option<ast_defs::ReadonlyKind>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub callconv: ast_defs::ParamKind,
    pub user_attributes: UserAttributes<Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "f_")]
#[repr(C)]
pub struct Fun_<Ex, En> {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: Pos,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub readonly_this: Option<ast_defs::ReadonlyKind>,
    pub annotation: En,
    /// Whether the return value is readonly
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub readonly_ret: Option<ast_defs::ReadonlyKind>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub ret: TypeHint<Ex>,
    pub params: Vec<FunParam<Ex, En>>,
    pub ctxs: Option<Contexts>,
    pub unsafe_ctxs: Option<Contexts>,
    pub body: FuncBody<Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub fun_kind: ast_defs::FunKind,
    pub user_attributes: UserAttributes<Ex, En>,
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct CaptureLid<Ex>(pub Ex, pub Lid);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "ef_")]
#[repr(C)]
pub struct Efun<Ex, En> {
    pub fun: Fun_<Ex, En>,
    pub use_: Vec<CaptureLid<Ex>>,
    pub closure_class_name: Option<String>,
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Targ<Ex>(pub Ex, pub Hint);

#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct CallExpr<Ex, En> {
    /// function
    pub func: Expr<Ex, En>,
    /// explicit type annotations
    pub targs: Vec<Targ<Ex>>,
    /// positional args, plus their calling convention
    pub args: Vec<(ast_defs::ParamKind, Expr<Ex, En>)>,
    /// unpacked arg
    pub unpacked_arg: Option<Expr<Ex, En>>,
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "fa_")]
#[repr(C)]
pub struct FileAttribute<Ex, En> {
    pub user_attributes: UserAttributes<Ex, En>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "tp_")]
#[repr(C)]
pub struct Tparam<Ex, En> {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub variance: ast_defs::Variance,
    pub name: Sid,
    pub parameters: Vec<Tparam<Ex, En>>,
    pub constraints: Vec<(ast_defs::ConstraintKind, Hint)>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub reified: ReifyKind,
    pub user_attributes: UserAttributes<Ex, En>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum EmitId {
    /// For globally defined type, the ID used in the .main function.
    #[rust_to_ocaml(name = "Emit_id")]
    EmitId(isize),
    /// Closures are hoisted to classes, but they don't get an entry in .main.
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "c_")]
#[repr(C)]
pub struct Class_<Ex, En> {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: Pos,
    pub annotation: En,
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub kind: ast_defs::ClassishKind,
    pub name: ClassName,
    /// The type parameters of a class A<T> (T is the parameter)
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub tparams: Vec<Tparam<Ex, En>>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub extends: Vec<ClassHint>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub uses: Vec<TraitHint>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub xhp_attr_uses: Vec<XhpAttrHint>,
    /// Categories are the equivalent of interfaces for XHP classes.
    /// These are either listed after the `implements` keyword, or as such
    /// within the XHP class:
    ///
    /// category cat1, cat2, cat3;
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub xhp_category: Option<(Pos, Vec<Pstring>)>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub reqs: Vec<ClassReq>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub implements: Vec<ClassHint>,
    pub where_constraints: Vec<WhereConstraintHint>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub consts: Vec<ClassConst<Ex, En>>,
    pub typeconsts: Vec<ClassTypeconstDef<Ex, En>>,
    /// a.k.a properties
    pub vars: Vec<ClassVar<Ex, En>>,
    pub methods: Vec<Method_<Ex, En>>,
    /// XHP allows an XHP class to declare which types are allowed
    /// as chidren using the `children` keyword.
    pub xhp_children: Vec<(Pos, XhpChild)>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub xhp_attrs: Vec<XhpAttr<Ex, En>>,
    pub namespace: Nsenv,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub user_attributes: UserAttributes<Ex, En>,
    pub file_attributes: Vec<FileAttribute<Ex, En>>,
    pub docs_url: Option<String>,
    pub enum_: Option<Enum_>,
    pub doc_comment: Option<DocComment>,
    pub emit_id: Option<EmitId>,
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ClassReq(pub ClassHint, pub RequireKind);

#[rust_to_ocaml(and)]
pub type ClassHint = Hint;

#[rust_to_ocaml(and)]
pub type TraitHint = Hint;

#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ClassConstKind<Ex, En> {
    /// CCAbstract represents the states
    ///     abstract const int X;
    ///     abstract const int Y = 4;
    /// The expr option is a default value
    CCAbstract(Option<Expr<Ex, En>>),
    /// CCConcrete represents
    ///     const int Z = 4;
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "cc_")]
#[repr(C)]
pub struct ClassConst<Ex, En> {
    pub user_attributes: UserAttributes<Ex, En>,
    pub type_: Option<Hint>,
    pub id: Sid,
    pub kind: ClassConstKind<Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: Pos,
    pub doc_comment: Option<DocComment>,
}

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
    FromOcamlRep,
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
pub struct ClassAbstractTypeconst {
    pub as_constraint: Option<Hint>,
    pub super_constraint: Option<Hint>,
    #[rust_to_ocaml(attr = "transform.explicit")]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "c_tconst_")]
#[repr(C)]
pub struct ClassTypeconstDef<Ex, En> {
    pub user_attributes: UserAttributes<Ex, En>,
    pub name: Sid,
    pub kind: ClassTypeconst,
    #[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "xai_")]
#[repr(C)]
pub struct XhpAttrInfo {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub like: Option<Pos>,
    pub tag: Option<XhpAttrTag>,
    #[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "cv_")]
#[repr(C)]
pub struct ClassVar<Ex, En> {
    pub final_: bool,
    pub xhp_attr: Option<XhpAttrInfo>,
    pub abstract_: bool,
    pub readonly: bool,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub visibility: Visibility,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub type_: TypeHint<Ex>,
    pub id: Sid,
    pub expr: Option<Expr<Ex, En>>,
    pub user_attributes: UserAttributes<Ex, En>,
    pub doc_comment: Option<DocComment>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    #[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "m_")]
#[repr(C)]
pub struct Method_<Ex, En> {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: Pos,
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub readonly_this: bool,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub visibility: Visibility,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, En>>,
    pub where_constraints: Vec<WhereConstraintHint>,
    pub params: Vec<FunParam<Ex, En>>,
    pub ctxs: Option<Contexts>,
    pub unsafe_ctxs: Option<Contexts>,
    pub body: FuncBody<Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub fun_kind: ast_defs::FunKind,
    pub user_attributes: UserAttributes<Ex, En>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub readonly_ret: Option<ast_defs::ReadonlyKind>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub ret: TypeHint<Ex>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<DocComment>,
}

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
pub type Nsenv = std::sync::Arc<namespace_env::Env>;

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "t_")]
#[repr(C)]
pub struct Typedef<Ex, En> {
    pub annotation: En,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, En>>,
    pub as_constraint: Option<Hint>,
    pub super_constraint: Option<Hint>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub kind: Hint,
    pub user_attributes: UserAttributes<Ex, En>,
    pub file_attributes: Vec<FileAttribute<Ex, En>>,
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: file_info::Mode,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub vis: TypedefVisibility,
    pub namespace: Nsenv,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: Pos,
    pub emit_id: Option<EmitId>,
    pub is_ctx: bool,
    pub internal: bool,
    pub module: Option<Sid>,
    pub docs_url: Option<String>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "cst_")]
#[repr(C)]
pub struct Gconst<Ex, En> {
    pub annotation: En,
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: file_info::Mode,
    pub name: Sid,
    pub type_: Option<Hint>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub value: Expr<Ex, En>,
    pub namespace: Nsenv,
    #[rust_to_ocaml(attr = "transform.opaque")]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "fd_")]
#[repr(C)]
pub struct FunDef<Ex, En> {
    pub namespace: Nsenv,
    pub file_attributes: Vec<FileAttribute<Ex, En>>,
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: file_info::Mode,
    pub name: Sid,
    pub fun: Fun_<Ex, En>,
    pub internal: bool,
    pub module: Option<Sid>,
    pub tparams: Vec<Tparam<Ex, En>>,
    pub where_constraints: Vec<WhereConstraintHint>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "md_")]
#[repr(C)]
pub struct ModuleDef<Ex, En> {
    pub annotation: En,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub name: ast_defs::Id,
    pub user_attributes: UserAttributes<Ex, En>,
    pub file_attributes: Vec<FileAttribute<Ex, En>>,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub span: Pos,
    #[rust_to_ocaml(attr = "visitors.opaque")]
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub mode: file_info::Mode,
    pub doc_comment: Option<DocComment>,
    pub exports: Option<Vec<MdNameKind>>,
    pub imports: Option<Vec<MdNameKind>>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum MdNameKind {
    MDNameGlobal(Pos),
    MDNamePrefix(Sid),
    MDNameExact(Sid),
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Def<Ex, En> {
    Fun(Box<FunDef<Ex, En>>),
    Class(Box<Class_<Ex, En>>),
    Stmt(Box<Stmt<Ex, En>>),
    Typedef(Box<Typedef<Ex, En>>),
    Constant(Box<Gconst<Ex, En>>),
    #[rust_to_ocaml(inline_tuple)]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
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

#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
pub type DocComment = ast_defs::Pstring;

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum ImportFlavor {
    Include,
    Require,
    IncludeOnce,
    RequireOnce,
}
impl TrivialDrop for ImportFlavor {}
arena_deserializer::impl_deserialize_in_arena!(ImportFlavor);

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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum XhpChild {
    ChildName(Sid),
    ChildList(Vec<XhpChild>),
    ChildUnary(Box<XhpChild>, XhpChildOp),
    ChildBinary(Box<XhpChild>, Box<XhpChild>),
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum XhpChildOp {
    ChildStar,
    ChildPlus,
    ChildQuestion,
}
impl TrivialDrop for XhpChildOp {}
arena_deserializer::impl_deserialize_in_arena!(XhpChildOp);

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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Hint(pub Pos, pub Box<Hint_>);

#[rust_to_ocaml(and)]
pub type VariadicHint = Option<Hint>;

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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct UserAttributes<Ex, En>(pub Vec<UserAttribute<Ex, En>>);

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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Contexts(pub Pos, pub Vec<Context>);

#[rust_to_ocaml(and)]
pub type Context = Hint;

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[rust_to_ocaml(prefix = "hfparam_")]
#[repr(C)]
pub struct HfParamInfo {
    pub kind: ast_defs::ParamKind,
    pub readonlyness: Option<ast_defs::ReadonlyKind>,
    pub optional: Option<ast_defs::OptionalKind>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "hf_")]
#[repr(C)]
pub struct HintFun {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub is_readonly: Option<ast_defs::ReadonlyKind>,
    pub param_tys: Vec<Hint>,
    pub param_info: Vec<Option<HfParamInfo>>,
    pub variadic_ty: VariadicHint,
    pub ctxs: Option<Contexts>,
    #[rust_to_ocaml(attr = "transform.explicit")]
    pub return_ty: Hint,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub is_readonly_return: Option<ast_defs::ReadonlyKind>,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Hint_ {
    Hprim(Tprim),
    Happly(ClassName, Vec<Hint>),
    Hoption(Hint),
    Hlike(Hint),
    Hfun(HintFun),
    Htuple(Vec<Hint>),
    #[rust_to_ocaml(name = "Hclass_args")]
    HclassArgs(Hint),
    Hshape(NastShapeInfo),
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
    Haccess(Hint, Vec<Sid>),
    Hsoft(Hint),
    Hrefinement(Hint, Vec<Refinement>),
    Hmixed,
    Hwildcard,
    Hnonnull,
    Habstr(String, Vec<Hint>),
    #[rust_to_ocaml(name = "Hvec_or_dict")]
    HvecOrDict(Option<Hint>, Hint),
    Hthis,
    Hdynamic,
    Hnothing,
    Hunion(Vec<Hint>),
    Hintersection(Vec<Hint>),
    #[rust_to_ocaml(name = "Hfun_context")]
    HfunContext(String),
    Hvar(String),
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Refinement {
    Rctx(Sid, CtxRefinement),
    Rtype(Sid, TypeRefinement),
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum TypeRefinement {
    TRexact(Hint),
    TRloose(TypeRefinementBounds),
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "tr_")]
#[repr(C)]
pub struct TypeRefinementBounds {
    pub lower: Vec<Hint>,
    pub upper: Vec<Hint>,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum CtxRefinement {
    CRexact(Hint),
    CRloose(CtxRefinementBounds),
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "cr_")]
#[repr(C)]
pub struct CtxRefinementBounds {
    pub lower: Option<Hint>,
    pub upper: Option<Hint>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "sfi_")]
#[repr(C)]
pub struct ShapeFieldInfo {
    pub optional: bool,
    pub hint: Hint,
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub name: ast_defs::ShapeFieldName,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "nsi_")]
#[repr(C)]
pub struct NastShapeInfo {
    pub allows_unknown_fields: bool,
    pub field_map: Vec<ShapeFieldInfo>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "visitors.opaque")]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum KvcKind {
    Map,
    ImmMap,
    Dict,
}
impl TrivialDrop for KvcKind {}
arena_deserializer::impl_deserialize_in_arena!(KvcKind);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "visitors.opaque")]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum VcKind {
    Vector,
    ImmVector,
    Vec,
    Set,
    ImmSet,
    Keyset,
}
impl TrivialDrop for VcKind {}
arena_deserializer::impl_deserialize_in_arena!(VcKind);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "e_")]
#[repr(C)]
pub struct Enum_ {
    pub base: Hint,
    pub constraint: Option<Hint>,
    pub includes: Vec<Hint>,
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
pub struct WhereConstraintHint(pub Hint, pub ast_defs::ConstraintKind, pub Hint);
