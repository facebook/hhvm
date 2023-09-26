(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Ensure that doc comments are always on a separate line by
 * requiring a lot of padding for inline doc comments. *)
[@@@ocamlformat "doc-comments-padding=80"]

(* We could open Hh_prelude here, but then we get errors about using ==,
   which some visitor below uses. *)
open Base.Export

type 'a local_id_map = 'a Local_id.Map.t [@@deriving eq, hash, ord, map, show]

let pp_local_id_map _ fmt map =
  Format.fprintf fmt "@[<hov 2>{";
  ignore
    (Local_id.Map.fold
       (fun key _ sep ->
         if sep then Format.fprintf fmt "@ ";
         Local_id.pp fmt key;
         true)
       map
       false);
  Format.fprintf fmt "}@]"

type pos = Ast_defs.pos [@@deriving eq, hash, show, ord] [@@transform.opaque]

let hash_pos hsv _pos = hsv

type byte_string = Ast_defs.byte_string
[@@deriving eq, hash, show, ord] [@@transform.opaque]

type visibility = Ast_defs.visibility =
  | Private
  | Public
  | Protected
  | Internal
[@@deriving eq, hash, ord, show { with_path = false }] [@@transform.opaque]

type tprim = Ast_defs.tprim =
  | Tnull
  | Tvoid
  | Tint
  | Tbool
  | Tfloat
  | Tstring
  | Tresource
  | Tnum
  | Tarraykey
  | Tnoreturn
[@@deriving eq, hash, ord, show { with_path = false }] [@@transform.opaque]

type typedef_visibility = Ast_defs.typedef_visibility =
  | Transparent
  | Opaque
  | OpaqueModule
  | CaseType
[@@deriving eq, hash, ord, show { with_path = false }] [@@transform.opaque]

type reify_kind = Ast_defs.reify_kind =
  | Erased
  | SoftReified
  | Reified
[@@deriving eq, hash, ord, show { with_path = false }] [@@transform.opaque]

type pstring = Ast_defs.pstring
[@@deriving eq, hash, ord, hash, show] [@@transform.opaque]

type positioned_byte_string = Ast_defs.positioned_byte_string
[@@deriving eq, ord, show]

type og_null_flavor = Ast_defs.og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe
[@@deriving eq, hash, ord, show { with_path = false }] [@@transform.opaque]

type prop_or_method = Ast_defs.prop_or_method =
  | Is_prop
  | Is_method
[@@deriving eq, hash, ord, show { with_path = false }] [@@transform.opaque]

type local_id = (Local_id.t[@visitors.opaque]) [@@transform.opaque]

and lid = pos * local_id [@@transform.opaque]

and sid = Ast_defs.id [@@transform.opaque]

and class_name = sid [@@transform.opaque]

(** Aast.program represents the top-level definitions in a Hack program.
 * ex: Expression annotation type (when typechecking, the inferred type)
 * en: Environment (tracking state inside functions and classes)
 *)
and ('ex, 'en) program = ('ex, 'en) def list

and ('ex, 'en) stmt = (pos[@transform.opaque]) * ('ex, 'en) stmt_

and ('ex, 'en) stmt_ =
  | Fallthrough
      (** Marker for a switch statement that falls through.
       *
       *     // FALLTHROUGH *)
  | Expr of ('ex, 'en) expr
      (** Standalone expression.
       *
       *     1 + 2; *)
  | Break
      (** Break inside a loop or switch statement.
       *
       *     break; *)
  | Continue
      (** Continue inside a loop or switch statement.
       *
       *     continue; *)
  | Throw of ('ex, 'en) expr
      (** Throw an exception.
       *
       * throw $foo; *)
  | Return of ('ex, 'en) expr option
      (** Return, with an optional value.
       *
       *     return;
       *     return $foo; *)
  | Yield_break
      (** Yield break, terminating the current generator. This behaves like
       * return; but is more explicit, and ensures the function is treated
       * as a generator.
       *
       *     yield break; *)
  | Awaitall of
      (* Temporaries assigned when running awaits. *)
      (lid * ('ex, 'en) expr) list
      * (* Block assigning temporary to relevant locals. *)
      ('ex, 'en) block
      (** Concurrent block. All the await expressions are awaited at the
       * same time, similar to genva().
       *
       * We store the desugared form. In the below example, the list is:
       * [('__tmp$1', f()), (__tmp$2, g()), (None, h())]
       * and the block assigns the temporary variables back to the locals.
       * { $foo = __tmp$1; $bar = __tmp$2; }
       *
       *     concurrent {
       *       $foo = await f();
       *       $bar = await g();
       *       await h();
       *     } *)
  | Concurrent of ('ex, 'en) block
      (** Concurrent block. All the await expressions are awaited at the
       * same time, similar to genva().
       *
       *     concurrent {
       *       $foo = await f();
       *       $bar = await g();
       *       await h();
       *     } *)
  | If of ('ex, 'en) expr * ('ex, 'en) block * ('ex, 'en) block
      (** If statement.
       *
       *     if ($foo) { ... } else { ... } *)
  | Do of ('ex, 'en) block * ('ex, 'en) expr
      (** Do-while loop.
       *
       *     do {
       *       bar();
       *     } while($foo) *)
  | While of ('ex, 'en) expr * ('ex, 'en) block
      (** While loop.
       *
       *     while ($foo) {
       *       bar();
       *     } *)
  | Using of ('ex, 'en) using_stmt
      (** Initialize a value that is automatically disposed of.
       *
       *     using $foo = bar(); // disposed at the end of the function
       *     using ($foo = bar(), $baz = quux()) {} // disposed after the block *)
  | For of
      ('ex, 'en) expr list
      * ('ex, 'en) expr option
      * ('ex, 'en) expr list
      * ('ex, 'en) block
      (** For loop. The initializer and increment parts can include
       * multiple comma-separated statements. The termination condition is
       * optional.
       *
       *     for ($i = 0; $i < 100; $i++) { ... }
       *     for ($x = 0, $y = 0; ; $x++, $y++) { ... } *)
  | Switch of
      ('ex, 'en) expr * ('ex, 'en) case list * ('ex, 'en) default_case option
      (** Switch statement.
       *
       *     switch ($foo) {
       *       case X:
       *         bar();
       *         break;
       *       default:
       *         baz();
       *         break;
       *     } *)
  | Match of ('ex, 'en) stmt_match
      (** Match statement.
       *
       *     match ($x) {
       *       _: FooClass => {
       *         foo($x);
       *       }
       *       _ => {
       *         bar();
       *       }
       *     } *)
  | Foreach of ('ex, 'en) expr * ('ex, 'en) as_expr * ('ex, 'en) block
      (** For-each loop.
       *
       *     foreach ($items as $item) { ... }
       *     foreach ($items as $key => value) { ... }
       *     foreach ($items await as $item) { ... } // AsyncIterator<_>
       *     foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_> *)
  | Try of ('ex, 'en) block * ('ex, 'en) catch list * ('ex, 'en) finally_block
      (** Try statement, with catch blocks and a finally block.
       *
       *     try {
       *       foo();
       *     } catch (SomeException $e) {
       *       bar();
       *     } finally {
       *       baz();
       *     } *)
  | Noop
      (** No-op, the empty statement.
       *
       *     {}
       *     while (true) ;
       *     if ($foo) {} // the else is Noop here *)
  | Declare_local of lid * hint * ('ex, 'en) expr option
      (** Declare a local variable with the given type and optional initial value *)
  | Block of lid list option * ('ex, 'en) block
      (** Block, a list of statements in curly braces.
       *
       *     { $foo = 42; }
       * If present, the optional list of identifiers are those that are scoped to this
       * Block, they will be unset upon exit to the block. *)
  | Markup of pstring
      [@transform.opaque]
      (** The mode tag at the beginning of a file.
       * TODO: this really belongs in def.
       *
       *     <?hh *)
  | AssertEnv of env_annot * ((pos * 'ex) local_id_map[@map.opaque])
      [@transform.opaque]
      (** Used in IFC to track type inference environments. Not user
       * denotable. *)

and env_annot =
  | Join
  | Refinement
[@@transform.opaque]

and ('ex, 'en) using_stmt = {
  us_is_block_scoped: bool;
  us_has_await: bool;
  us_exprs: (pos[@transform.opaque]) * ('ex, 'en) expr list;
  us_block: ('ex, 'en) block;
}

and ('ex, 'en) as_expr =
  | As_v of ('ex, 'en) expr
  | As_kv of ('ex, 'en) expr * ('ex, 'en) expr
  | Await_as_v of (pos[@transform.opaque]) * ('ex, 'en) expr
  | Await_as_kv of (pos[@transform.opaque]) * ('ex, 'en) expr * ('ex, 'en) expr

and ('ex, 'en) block = ('ex, 'en) stmt list

and ('ex, 'en) finally_block = ('ex, 'en) stmt list

and ('ex, 'en) stmt_match = {
  sm_expr: ('ex, 'en) expr;
  sm_arms: ('ex, 'en) stmt_match_arm list;
}

and ('ex, 'en) stmt_match_arm = {
  sma_pat: pattern;
  sma_body: ('ex, 'en) block;
}

and pattern =
  | PVar of pat_var
      (** Variable patterns *)
  | PRefinement of pat_refinement
      (** Refinement patterns *)

and pat_var = {
  pv_pos: pos; [@transform.opaque]
  pv_id: lid option;
}

and pat_refinement = {
  pr_pos: pos; [@transform.opaque]
  pr_id: lid option;
  pr_hint: hint;
}

and ('ex, 'en) class_id = 'ex * (pos[@transform.opaque]) * ('ex, 'en) class_id_

(** Class ID, used in things like instantiation and static property access. *)
and ('ex, 'en) class_id_ =
  | CIparent
      (** The class ID of the parent of the lexically scoped class.
       *
       * In a trait, it is the parent class ID of the using class.
       *
       *     parent::some_meth()
       *     parent::$prop = 1;
       *     new parent(); *)
  | CIself
      (** The class ID of the lexically scoped class.
       *
       * In a trait, it is the class ID of the using class.
       *
       *     self::some_meth()
       *     self::$prop = 1;
       *     new self(); *)
  | CIstatic
      (** The class ID of the late static bound class.
       *
       * https://www.php.net/manual/en/language.oop5.late-static-bindings.php
       *
       * In a trait, it is the late static bound class ID of the using class.
       *
       *     static::some_meth()
       *     static::$prop = 1;
       *     new static(); *)
  | CIexpr of ('ex, 'en) expr
      (** Dynamic class name.
       *
       * TODO: Syntactically this can only be an Lvar/This/Lplaceholder.
       * We should use lid rather than expr.
       *
       *     // Assume $d has type dynamic.
       *     $d::some_meth();
       *     $d::$prop = 1;
       *     new $d(); *)
  | CI of class_name
      (** Explicit class name. This is the common case.
       *
       *     Foo::some_meth()
       *     Foo::$prop = 1;
       *     new Foo(); *)

and ('ex, 'en) expr = 'ex * (pos[@transform.opaque]) * ('ex, 'en) expr_

and 'ex collection_targ =
  | CollectionTV of 'ex targ
  | CollectionTKV of 'ex targ * 'ex targ

and ('ex, 'en) function_ptr_id =
  | FP_id of sid
  | FP_class_const of ('ex, 'en) class_id * (pstring[@transform.opaque])
      (** An expression tree literal consists of a hint, splices, and
 *  expressions. Consider this example:
 *
 *     Foo`1 + ${$x} + ${bar()}` *)

and ('ex, 'en) expression_tree = {
  et_hint: hint;
      (** The hint before the backtick, so Foo in this example. *)
  et_splices: ('ex, 'en) stmt list;
      (** The values spliced into expression tree at runtime are assigned
       * to temporaries.
       *
       *     $0tmp1 = $x; $0tmp2 = bar(); *)
  et_function_pointers: ('ex, 'en) stmt list;
      (** The list of global functions and static methods assigned to
       * temporaries.
       *
       *     $0fp1 = foo<>; *)
  et_virtualized_expr: ('ex, 'en) expr;
      (** The expression that gets type checked.
       *
       *     1 + $0tmp1 + $0tmp2 *)
  et_runtime_expr: ('ex, 'en) expr;
      (** The expression that's executed at runtime.
       *
       *     Foo::makeTree($v ==> $v->visitBinOp(...)) *)
  et_dollardollar_pos: pos option; [@transform.opaque]
      (** Position of the first $$ in a splice that refers
       * to a variable outside the Expression Tree
       *
       *     $x |> Code`${ $$ }` // Pos of the $$
       *     Code`${ $x |> foo($$) }` // None *)
}

and ('ex, 'en) expr_ =
  | Darray of
      ('ex targ * 'ex targ) option * (('ex, 'en) expr * ('ex, 'en) expr) list
      (** darray literal.
       *
       *     darray['x' => 0, 'y' => 1]
       *     darray<string, int>['x' => 0, 'y' => 1] *)
  | Varray of 'ex targ option * ('ex, 'en) expr list
      (** varray literal.
       *
       *     varray['hello', 'world']
       *     varray<string>['hello', 'world'] *)
  | Shape of
      ((Ast_defs.shape_field_name[@transform.opaque]) * ('ex, 'en) expr) list
      (** Shape literal.
       *
       *     shape('x' => 1, 'y' => 2)*)
  | ValCollection of
      ((pos * vc_kind)[@transform.opaque])
      * 'ex targ option
      * ('ex, 'en) expr list
      (** Collection literal for indexable structures.
       *
       *     Vector {1, 2}
       *     ImmVector {}
       *     Set<string> {'foo', 'bar'}
       *     vec[1, 2]
       *     keyset[] *)
  | KeyValCollection of
      ((pos * kvc_kind)[@transform.opaque])
      * ('ex targ * 'ex targ) option
      * ('ex, 'en) field list
      (** Collection literal for key-value structures.
       *
       *     dict['x' => 1, 'y' => 2]
       *     Map<int, string> {}
       *     ImmMap {} *)
  | Null
      (** Null literal.
       *
       *     null *)
  | This
      (** The local variable representing the current class instance.
       *
       *     $this *)
  | True
      (** Boolean literal.
       *
       *     true *)
  | False
      (** Boolean literal.
       *
       *     false *)
  | Omitted
      (** The empty expression.
       *
       *     list(, $y) = vec[1, 2] // Omitted is the first expression inside list() *)
  | Invalid of ('ex, 'en) expr option
      (** Invalid expression marker generated during elaboration / validation phases
       *
       *     class MyFoo {
       *       const int BAR = calls_are_invalid_here();
       *     }
       *)
  | Id of sid
      (** An identifier. Used for method names and global constants.
       *
       *     SOME_CONST
       *     $x->foo() // id: "foo" *)
  | Lvar of lid
      (** Local variable.
       *
       *     $foo *)
  | Dollardollar of lid
      (** The extra variable in a pipe expression.
       *
       *     $$ *)
  | Clone of ('ex, 'en) expr
      (** Clone expression.
       *
       *     clone $foo *)
  | Array_get of ('ex, 'en) expr * ('ex, 'en) expr option
      (** Array indexing.
       *
       *     $foo[]
       *     $foo[$bar] *)
  | Obj_get of
      ('ex, 'en) expr
      * ('ex, 'en) expr
      * (og_null_flavor[@transform.opaque])
      * (prop_or_method[@transform.opaque])
      (** Instance property or method access.
       *
       *     $foo->bar      // OG_nullthrows, Is_prop: access named property
       *     ($foo->bar)()  // OG_nullthrows, Is_prop: call lambda stored in named property
       *     $foo?->bar     // OG_nullsafe,   Is_prop
       *     ($foo?->bar)() // OG_nullsafe,   Is_prop
       *
       *     $foo->bar()    // OG_nullthrows, Is_method: call named method
       *     $foo->$bar()   // OG_nullthrows, Is_method: dynamic call, method name stored in local $bar
       *     $foo?->bar()   // OG_nullsafe,   Is_method
       *     $foo?->$bar()  // OG_nullsafe,   Is_method
       *
       * prop_or_method is:
       *   - Is_prop for property access
       *   - Is_method for method call, only possible when the node is the receiver in a Call node.
       *
       *)
  | Class_get of
      ('ex, 'en) class_id
      * ('ex, 'en) class_get_expr
      * (prop_or_method[@transform.opaque])
      (** Static property or dynamic method access. The rhs of the :: begins
       * with $ or is some non-name expression appearing within braces {}.
       *
       *     Foo::$bar               // Is_prop: access named static property
       *     Foo::{$bar}             // Is_prop
       *     (Foo::$bar)();          // Is_prop: call lambda stored in static property Foo::$bar
       *     $classname::$bar        // Is_prop
       *
       *     Foo::$bar();            // Is_method: dynamic call, method name stored in local $bar
       *     Foo::{$bar}();          // Is_method
       *)
  | Class_const of ('ex, 'en) class_id * (pstring[@transform.opaque])
      (** Class constant or static method call. As a standalone expression,
       * this is a class constant. Inside a Call node, this is a static
       * method call. The rhs of the :: does not begin with $ or is a name
       * appearing within braces {}.
       *
       * This is not ambiguous, because constants are not allowed to
       * contain functions.
       *
       *     Foo::some_const            // Const
       *     Foo::{another_const}       // Const: braces are elided
       *     Foo::class                 // Const: fully qualified class name of Foo
       *     Foo::staticMeth()          // Call
       *     $classname::staticMeth()   // Call
       *
       * This syntax is used for both static and instance methods when
       * calling the implementation on the superclass.
       *
       *     parent::someStaticMeth()
       *     parent::someInstanceMeth()
       *)
  | Call of ('ex, 'en) call_expr
      (** Function or method call.
       *
       *     foo()
       *     $x()
       *     foo<int>(1, 2, ...$rest)
       *     $x->foo()
       *     bar(inout $x);
       *     foobar(inout $x[0])
       *
       *     async { return 1; }
       *     // lowered to:
       *     (async () ==> { return 1; })() *)
  | FunctionPointer of ('ex, 'en) function_ptr_id * 'ex targ list
      (** A reference to a function or method.
       *
       *     foo_fun<>
       *     FooCls::meth<int> *)
  | Int of string
      (** Integer literal.
       *
       *     42
       *     0123 // octal
       *     0xBEEF // hexadecimal
       *     0b11111111 // binary *)
  | Float of string
      (** Float literal.
       *
       *     1.0
       *     1.2e3
       *     7E-10 *)
  | String of byte_string
      [@transform.opaque]
      (** String literal.
       *
       *     "foo"
       *     'foo'
       *
       *     <<<DOC
       *     foo
       *     DOC
       *
       *     <<<'DOC'
       *     foo
       *     DOC *)
  | String2 of ('ex, 'en) expr list
      (** Interpolated string literal.
       *
       *     "hello $foo $bar"
       *
       *     <<<DOC
       *     hello $foo $bar
       *     DOC *)
  | PrefixedString of string * ('ex, 'en) expr
      (** Prefixed string literal. Only used for regular expressions.
       *
       *     re"foo" *)
  | Yield of ('ex, 'en) afield
      (** Yield expression. The enclosing function should have an Iterator
       * return type.
       *
       *     yield $foo // enclosing function returns an Iterator
       *     yield $foo => $bar // enclosing function returns a KeyedIterator *)
  | Await of ('ex, 'en) expr
      (** Await expression.
       *
       *     await $foo *)
  | ReadonlyExpr of ('ex, 'en) expr
      (** Readonly expression.
       *
       *     readonly $foo *)
  | Tuple of ('ex, 'en) expr list
      (** Tuple expression.
       *
       *     tuple("a", 1, $foo) *)
  | List of ('ex, 'en) expr list
      (** List expression, only used in destructuring. Allows any arbitrary
       * lvalue as a subexpression. May also nest.
       *
       *     list($x, $y) = vec[1, 2];
       *     list(, $y) = vec[1, 2]; // skipping items
       *     list(list($x)) = vec[vec[1]]; // nesting
       *     list($v[0], $x[], $y->foo) = $blah; *)
  | Cast of hint * ('ex, 'en) expr
      (** Cast expression, converting a value to a different type. Only
       * primitive types are supported in the hint position.
       *
       *     (int)$foo
       *     (string)$foo *)
  | Unop of (Ast_defs.uop[@transform.opaque]) * ('ex, 'en) expr
      (** Unary operator.
       *
       *     !$foo
       *     -$foo
       *     +$foo
       *     $foo++ *)
  | Binop of ('ex, 'en) binop
      (** Binary operator.
       *
       *     $foo + $bar *)
  | Pipe of lid * ('ex, 'en) expr * ('ex, 'en) expr
      (** Pipe expression. The lid is the ID of the $$ that is implicitly
       * declared by this pipe.
       *
       * See also Dollardollar.
       *
       *     foo() |> bar(1, $$) // equivalent: bar(1, foo())
       *
       * $$ is not required on the RHS of pipe expressions, but it's
       * pretty pointless to use pipes without $$.
       *
       *     foo() |> bar(); // equivalent: foo(); bar(); *)
  | Eif of ('ex, 'en) expr * ('ex, 'en) expr option * ('ex, 'en) expr
      (** Ternary operator, or elvis operator.
       *
       *     $foo ? $bar : $baz // ternary
       *     $foo ?: $baz // elvis *)
  | Is of ('ex, 'en) expr * hint
      (** Is operator.
       *
       *     $foo is SomeType *)
  | As of ('ex, 'en) expr * hint * (* is nullable *) bool
      (** As operator.
       *
       *     $foo as int
       *     $foo ?as int *)
  | Upcast of ('ex, 'en) expr * hint
      (** Upcast operator.
       *
       *     $foo : int *)
  | New of
      ('ex, 'en) class_id
      * 'ex targ list
      * ('ex, 'en) expr list
      * ('ex, 'en) expr option
      * (* constructor *)
      'ex
      (** Instantiation.
       *
       *     new Foo(1, 2);
       *     new Foo<int, T>();
       *     new Foo('blah', ...$rest); *)
  | Efun of ('ex, 'en) efun
      (** PHP-style lambda. Does not capture variables unless explicitly
       * specified.
       *
       * Mnemonic: 'expanded lambda', since we can desugar Lfun to Efun.
       *
       *     function($x) { return $x; }
       *     function(int $x): int { return $x; }
       *     function($x) use ($y) { return $y; }
       *     function($x): int use ($y, $z) { return $x + $y + $z; } *)
  | Lfun of ('ex, 'en) fun_ * 'ex capture_lid list
      (** Hack lambda. Captures variables automatically.
       *
       *     $x ==> $x
       *     (int $x): int ==> $x + $other
       *     ($x, $y) ==> { return $x + $y; } *)
  | Xml of
      (class_name[@transform.opaque])
      * ('ex, 'en) xhp_attribute list
      * ('ex, 'en) expr list
      (** XHP expression. May contain interpolated expressions.
       *
       *     <foo x="hello" y={$foo}>hello {$bar}</foo> *)
  | Import of (import_flavor[@transform.opaque]) * ('ex, 'en) expr
      (** Include or require expression.
       *
       *     require('foo.php')
       *     require_once('foo.php')
       *     include('foo.php')
       *     include_once('foo.php') *)
  | Collection of
      (class_name[@transform.opaque])
      * 'ex collection_targ option
      * ('ex, 'en) afield list
      (** Collection literal.
       *
       * TODO: T38184446 this is redundant with ValCollection/KeyValCollection.
       *
       *     Vector {} *)
  | ExpressionTree of ('ex, 'en) expression_tree
      (** Expression tree literal. Expression trees are not evaluated at
       * runtime, but desugared to an expression representing the code.
       *
       *     Foo`1 + bar()`
       *     Foo`(() ==> { while(true) {} })()` // not an infinite loop at runtime
       *
       * Splices are evaluated as normal Hack code. The following two expression trees
       * are equivalent. See also `ET_Splice`.
       *
       *     Foo`1 + ${do_stuff()}`
       *
       *     $x = do_stuff();
       *     Foo`1 + ${$x}` *)
  | Lplaceholder of pos
      [@transform.opaque]
      (** Placeholder local variable.
       *
       *     $_ *)
  | Method_caller of class_name * (pstring[@transform.opaque])
      (** Instance method reference that can be called with an instance.
       *
       *     meth_caller(FooClass::class, 'some_meth')
       *     meth_caller('FooClass', 'some_meth')
       *
       * These examples are equivalent to:
       *
       *     (FooClass $f, ...$args) ==> $f->some_meth(...$args) *)
  | Pair of ('ex targ * 'ex targ) option * ('ex, 'en) expr * ('ex, 'en) expr
      (** Pair literal.
       *
       *     Pair {$foo, $bar} *)
  | ET_Splice of ('ex, 'en) expr
      (** Expression tree splice expression. Only valid inside an
       * expression tree literal (backticks). See also `ExpressionTree`.
       *
       *     ${$foo} *)
  | EnumClassLabel of class_name option * string
      (** Label used for enum classes.
       *
       *     enum_name#label_name or #label_name *)
  | Hole of ('ex, 'en) expr * 'ex * 'ex * hole_source
      (** Annotation used to record failure in subtyping or coercion of an
       * expression and calls to [unsafe_cast] or [enforced_cast].
       *
       * The [hole_source] indicates whether this came from an
       * explicit call to [unsafe_cast] or [enforced_cast] or was
       * generated during typing.
       *
       * Given a call to [unsafe_cast]:
       * ```
       * function f(int $x): void { /* ... */ }
       *
       * function g(float $x): void {
       *    f(unsafe_cast<float,int>($x));
       * }
       * ```
       * After typing, this is represented by the following TAST fragment
       * ```
       * Call
       *   ( ( (..., function(int $x): void), Id (..., "\f"))
       *   , []
       *   , [ ( (..., int)
       *       , Hole
       *           ( ((..., float), Lvar (..., $x))
       *           , float
       *           , int
       *           , UnsafeCast
       *           )
       *       )
       *     ]
       *   , None
       *   )
       * ```
       *)
  | Package of sid
      (** Expression used to check whether a package exists.
       *
       *     package package-name *)

and hole_source =
  | Typing
  | UnsafeCast of hint list
  | UnsafeNonnullCast
  | EnforcedCast of hint list

and ('ex, 'en) binop = {
  bop: Ast_defs.bop; [@transform.opaque]
  lhs: ('ex, 'en) expr; [@transform.explicit]
  rhs: ('ex, 'en) expr; [@transform.explicit]
}

and ('ex, 'en) class_get_expr =
  | CGstring of (pstring[@transform.opaque])
  | CGexpr of ('ex, 'en) expr

and ('ex, 'en) case = ('ex, 'en) expr * ('ex, 'en) block

and ('ex, 'en) default_case = (pos[@transform.opaque]) * ('ex, 'en) block

and ('ex, 'en) gen_case =
  | Case of ('ex, 'en) case
  | Default of ('ex, 'en) default_case

and ('ex, 'en) catch = class_name * lid * ('ex, 'en) block

and ('ex, 'en) field = ('ex, 'en) expr * ('ex, 'en) expr

and ('ex, 'en) afield =
  | AFvalue of ('ex, 'en) expr
  | AFkvalue of ('ex, 'en) expr * ('ex, 'en) expr

and ('ex, 'en) xhp_simple = {
  xs_name: pstring; [@transform.opaque]
  xs_type: 'ex;
  xs_expr: ('ex, 'en) expr;
}

and ('ex, 'en) xhp_attribute =
  | Xhp_simple of ('ex, 'en) xhp_simple
  | Xhp_spread of ('ex, 'en) expr

and is_variadic = bool [@@transform.opaque]

and ('ex, 'en) fun_param = {
  param_annotation: 'ex;
  param_type_hint: 'ex type_hint;
  param_is_variadic: is_variadic;
  param_pos: pos; [@transform.opaque]
  param_name: string;
  param_expr: ('ex, 'en) expr option;
  param_readonly: Ast_defs.readonly_kind option; [@transform.opaque]
  param_callconv: Ast_defs.param_kind; [@transform.opaque]
  param_user_attributes: ('ex, 'en) user_attributes;
  param_visibility: visibility option; [@transform.opaque]
}

and ('ex, 'en) fun_ = {
  f_span: pos; [@transform.opaque]
  f_readonly_this: Ast_defs.readonly_kind option; [@transform.opaque]
  f_annotation: 'en;
  f_readonly_ret: Ast_defs.readonly_kind option; [@transform.opaque]
      (** Whether the return value is readonly *)
  f_ret: 'ex type_hint; [@transform.explicit]
  f_params: ('ex, 'en) fun_param list;
  f_ctxs: contexts option;
  f_unsafe_ctxs: contexts option;
  f_body: ('ex, 'en) func_body;
  f_fun_kind: Ast_defs.fun_kind; [@transform.opaque]
  f_user_attributes: ('ex, 'en) user_attributes;
  f_external: bool;
      (** true if this declaration has no body because it is an
       * external function declaration (e.g. from an HHI file) *)
  f_doc_comment: doc_comment option;
}

and 'ex capture_lid = 'ex * lid

and ('ex, 'en) efun = {
  ef_fun: ('ex, 'en) fun_;
  ef_use: 'ex capture_lid list;
  ef_closure_class_name: string option;
}

(**
 * Naming has two phases and the annotation helps to indicate the phase.
 * In the first pass, it will perform naming on everything except for function
 * and method bodies and collect information needed. Then, another round of
 * naming is performed where function bodies are named.
 *)
and ('ex, 'en) func_body = { fb_ast: ('ex, 'en) block }

(** A type annotation is two things:
 * - the localized hint, or if the hint is missing, the inferred type
 * - The typehint associated to this expression if it exists *)
and 'ex type_hint = 'ex * type_hint_

(** Explicit type argument to function, constructor, or collection literal.
 * 'ex = unit in NAST
 * 'ex = Typing_defs.(locl ty) in TAST,
 * and is used to record inferred type arguments, with wildcard hint.
 *)
and 'ex targ = 'ex * hint

and type_hint_ = hint option

and ('ex, 'en) call_expr = {
  func: ('ex, 'en) expr;
      (** function *)
  targs: 'ex targ list;
      (** explicit type annotations *)
  args: ((Ast_defs.param_kind[@transform.opaque]) * ('ex, 'en) expr) list;
      (** positional args, plus their calling convention *)
  unpacked_arg: ('ex, 'en) expr option;
      (** unpacked arg *)
}

and ('ex, 'en) user_attribute = {
  ua_name: sid;
  ua_params: ('ex, 'en) expr list;
      (** user attributes are restricted to scalar values *)
}

and ('ex, 'en) file_attribute = {
  fa_user_attributes: ('ex, 'en) user_attributes;
  fa_namespace: nsenv;
}

and ('ex, 'en) tparam = {
  tp_variance: Ast_defs.variance; [@transform.opaque]
  tp_name: sid;
  tp_parameters: ('ex, 'en) tparam list;
  tp_constraints: ((Ast_defs.constraint_kind[@transform.opaque]) * hint) list;
  tp_reified: reify_kind; [@transform.opaque]
  tp_user_attributes: ('ex, 'en) user_attributes;
}

and require_kind =
  | RequireExtends
  | RequireImplements
  | RequireClass
[@@transform.opaque]

and emit_id =
  | Emit_id of int
      (** For globally defined type, the ID used in the .main function. *)
  | Anonymous
      (** Closures are hoisted to classes, but they don't get an entry in .main. *)
[@@transform.opaque]

and ('ex, 'en) class_ = {
  c_span: pos; [@transform.opaque]
  c_annotation: 'en;
  c_mode: FileInfo.mode; [@visitors.opaque] [@transform.opaque]
  c_final: bool;
  c_is_xhp: bool;
  c_has_xhp_keyword: bool;
  c_kind: Ast_defs.classish_kind; [@transform.opaque]
  c_name: class_name;
  c_tparams: ('ex, 'en) tparam list; [@transform.explicit]
      (** The type parameters of a class A<T> (T is the parameter) *)
  c_extends: class_hint list; [@transform.explicit]
  c_uses: trait_hint list; [@transform.explicit]
  c_xhp_attr_uses: xhp_attr_hint list; [@transform.explicit]
  c_xhp_category: (pos * pstring list) option; [@transform.opaque]
      (** Categories are the equivalent of interfaces for XHP classes.
        These are either listed after the `implements` keyword, or as such
        within the XHP class:

          category cat1, cat2, cat3;
        *)
  c_reqs: class_req list; [@transform.explicit]
  c_implements: class_hint list; [@transform.explicit]
  c_where_constraints: where_constraint_hint list;
  c_consts: ('ex, 'en) class_const list; [@transform.explicit]
  c_typeconsts: ('ex, 'en) class_typeconst_def list;
  c_vars: ('ex, 'en) class_var list;
      (** a.k.a properties *)
  c_methods: ('ex, 'en) method_ list;
  c_xhp_children: ((pos[@transform.opaque]) * xhp_child) list;
      (** XHP allows an XHP class to declare which types are allowed
        as chidren using the `children` keyword. *)
  c_xhp_attrs: ('ex, 'en) xhp_attr list; [@transform.explicit]
  c_namespace: nsenv;
  c_user_attributes: ('ex, 'en) user_attributes; [@transform.explicit]
  c_file_attributes: ('ex, 'en) file_attribute list;
  c_docs_url: string option;
  c_enum: enum_ option;
  c_doc_comment: doc_comment option;
  c_emit_id: emit_id option;
  c_internal: bool;
  c_module: sid option;
}

and class_req = class_hint * require_kind

and class_hint = hint

and trait_hint = hint

and xhp_attr_hint = hint

and xhp_attr_tag =
  | Required
  | LateInit
[@@transform.opaque]

and ('ex, 'en) xhp_attr =
  'ex type_hint
  * ('ex, 'en) class_var
  * xhp_attr_tag option
  * ((pos[@transform.opaque]) * ('ex, 'en) expr list) option

and ('ex, 'en) class_const_kind =
  | CCAbstract of ('ex, 'en) expr option
      (** CCAbstract represents the states
       *     abstract const int X;
       *     abstract const int Y = 4;
       * The expr option is a default value
       *)
  | CCConcrete of ('ex, 'en) expr
      (** CCConcrete represents
       *     const int Z = 4;
       * The expr is the value of the constant. It is not optional
       *)

and ('ex, 'en) class_const = {
  cc_user_attributes: ('ex, 'en) user_attributes;
  cc_type: hint option;
  cc_id: sid;
  cc_kind: ('ex, 'en) class_const_kind;
  cc_span: pos; [@transform.opaque]
  cc_doc_comment: doc_comment option;
}

(** This represents a type const definition. If a type const is abstract then
 * then the type hint acts as a constraint. Any concrete definition of the
 * type const must satisfy the constraint.
 *
 * If the type const is not abstract then a type must be specified.
 *)
and class_abstract_typeconst = {
  c_atc_as_constraint: hint option;
  c_atc_super_constraint: hint option;
  c_atc_default: hint option; [@transform.explicit]
}

and class_concrete_typeconst = { c_tc_type: hint }

and class_typeconst =
  | TCAbstract of class_abstract_typeconst
  | TCConcrete of class_concrete_typeconst

and ('ex, 'en) class_typeconst_def = {
  c_tconst_user_attributes: ('ex, 'en) user_attributes;
  c_tconst_name: sid;
  c_tconst_kind: class_typeconst;
  c_tconst_span: pos; [@transform.opaque]
  c_tconst_doc_comment: doc_comment option;
  c_tconst_is_ctx: bool;
}

and xhp_attr_info = {
  xai_like: pos option; [@transform.opaque]
  xai_tag: xhp_attr_tag option;
  xai_enum_values: Ast_defs.xhp_enum_value list; [@transform.opaque]
}

and ('ex, 'en) class_var = {
  cv_final: bool;
  cv_xhp_attr: xhp_attr_info option;
  cv_abstract: bool;
  cv_readonly: bool;
  cv_visibility: visibility; [@transform.opaque]
  cv_type: 'ex type_hint; [@transform.explicit]
  cv_id: sid;
  cv_expr: ('ex, 'en) expr option;
  cv_user_attributes: ('ex, 'en) user_attributes;
  cv_doc_comment: doc_comment option;
  cv_is_promoted_variadic: bool;
  cv_is_static: bool;
  cv_span: pos; [@transform.opaque]
}

and ('ex, 'en) method_ = {
  m_span: pos; [@transform.opaque]
  m_annotation: 'en;
  m_final: bool;
  m_abstract: bool;
  m_static: bool;
  m_readonly_this: bool;
  m_visibility: visibility; [@transform.opaque]
  m_name: sid;
  m_tparams: ('ex, 'en) tparam list;
  m_where_constraints: where_constraint_hint list;
  m_params: ('ex, 'en) fun_param list;
  m_ctxs: contexts option;
  m_unsafe_ctxs: contexts option;
  m_body: ('ex, 'en) func_body;
  m_fun_kind: Ast_defs.fun_kind; [@transform.opaque]
  m_user_attributes: ('ex, 'en) user_attributes;
  m_readonly_ret: Ast_defs.readonly_kind option; [@transform.opaque]
  m_ret: 'ex type_hint; [@transform.explicit]
  m_external: bool;
      (** true if this declaration has no body because it is an external method
       * declaration (e.g. from an HHI file) *)
  m_doc_comment: doc_comment option;
}

and nsenv = (Namespace_env.env[@visitors.opaque]) [@@transform.opaque]

and ('ex, 'en) typedef = {
  t_annotation: 'en;
  t_name: sid;
  t_tparams: ('ex, 'en) tparam list;
  t_as_constraint: hint option;
  t_super_constraint: hint option;
  t_kind: hint; [@transform.explicit]
  t_user_attributes: ('ex, 'en) user_attributes;
  t_file_attributes: ('ex, 'en) file_attribute list;
  t_mode: FileInfo.mode; [@visitors.opaque] [@transform.opaque]
  t_vis: typedef_visibility; [@transform.opaque]
  t_namespace: nsenv;
  t_span: pos; [@transform.opaque]
  t_emit_id: emit_id option;
  t_is_ctx: bool;
  t_internal: bool;
  t_module: sid option;
  t_docs_url: string option;
  t_doc_comment: doc_comment option;
}

and ('ex, 'en) gconst = {
  cst_annotation: 'en;
  cst_mode: FileInfo.mode; [@visitors.opaque] [@transform.opaque]
  cst_name: sid;
  cst_type: hint option;
  cst_value: ('ex, 'en) expr; [@transform.explicit]
  cst_namespace: nsenv;
  cst_span: pos; [@transform.opaque]
  cst_emit_id: emit_id option;
}

and ('ex, 'en) fun_def = {
  fd_namespace: nsenv;
  fd_file_attributes: ('ex, 'en) file_attribute list;
  fd_mode: FileInfo.mode; [@visitors.opaque] [@transform.opaque]
  fd_name: sid;
  fd_fun: ('ex, 'en) fun_;
  fd_internal: bool;
  fd_module: sid option;
  fd_tparams: ('ex, 'en) tparam list;
  fd_where_constraints: where_constraint_hint list;
}

and ('ex, 'en) module_def = {
  md_annotation: 'en;
  md_name: Ast_defs.id; [@transform.opaque]
  md_user_attributes: ('ex, 'en) user_attributes;
  md_file_attributes: ('ex, 'en) file_attribute list;
  md_span: pos; [@transform.opaque]
  md_mode: FileInfo.mode; [@visitors.opaque] [@transform.opaque]
  md_doc_comment: doc_comment option;
  md_exports: md_name_kind list option;
  md_imports: md_name_kind list option;
}

and md_name_kind =
  | MDNameGlobal of pos
  | MDNamePrefix of sid
  | MDNameExact of sid
[@@transform.opaque]

and ('ex, 'en) def =
  | Fun of ('ex, 'en) fun_def
  | Class of ('ex, 'en) class_
  | Stmt of ('ex, 'en) stmt
  | Typedef of ('ex, 'en) typedef
  | Constant of ('ex, 'en) gconst
  | Namespace of sid * ('ex, 'en) def list
  | NamespaceUse of (ns_kind * sid * sid) list
  | SetNamespaceEnv of nsenv
  | FileAttributes of ('ex, 'en) file_attribute
  | Module of ('ex, 'en) module_def
  | SetModule of sid

and ns_kind =
  | NSNamespace
  | NSClass
  | NSClassAndNamespace
  | NSFun
  | NSConst
[@@transform.opaque]

and doc_comment = (Ast_defs.pstring[@visitors.opaque]) [@@transform.opaque]

and import_flavor =
  | Include
  | Require
  | IncludeOnce
  | RequireOnce
[@@transform.opaque]

and xhp_child =
  | ChildName of sid
  | ChildList of xhp_child list
  | ChildUnary of xhp_child * xhp_child_op
  | ChildBinary of xhp_child * xhp_child

and xhp_child_op =
  | ChildStar
  | ChildPlus
  | ChildQuestion
[@@transform.opaque]

and hint = (pos[@transform.opaque]) * hint_

and variadic_hint = hint option

and ('ex, 'en) user_attributes = ('ex, 'en) user_attribute list

and contexts = (pos[@transform.opaque]) * context list

and context = hint

and hf_param_info = {
  hfparam_kind: Ast_defs.param_kind;
  hfparam_readonlyness: Ast_defs.readonly_kind option;
}
[@@transform.opaque]

and hint_fun = {
  hf_is_readonly: Ast_defs.readonly_kind option; [@transform.opaque]
  hf_param_tys: hint list;
  (* hf_param_info is None when all three are none, for perf optimization reasons.
     It is not semantically incorrect for the record to appear with 3 None values,
     but in practice we shouldn't lower to that, since it wastes CPU/space *)
  hf_param_info: hf_param_info option list;
  hf_variadic_ty: variadic_hint;
  hf_ctxs: contexts option;
  hf_return_ty: hint; [@transform.explicit]
  hf_is_readonly_return: Ast_defs.readonly_kind option; [@transform.opaque]
}

and hint_ =
  | Hoption of hint
  | Hlike of hint
  | Hfun of hint_fun
  | Htuple of hint list
  | Happly of class_name * hint list
  | Hshape of nast_shape_info
  | Haccess of hint * sid list
      (** Accessing a type constant. Type constants are accessed like normal
       * class constants, but in type positions.
       *
       * SomeClass::TFoo
       * self::TFoo
       * this::TFoo
       *
       * Type constants can be also be chained, hence the list as the second
       * argument:
       *
       * SomeClass::TFoo::TBar // Haccess (Happly "SomeClass", ["TFoo", "TBar"])
       *
       * When using contexts, the receiver may be a variable rather than a
       * specific type:
       *
       * function uses_const_ctx(SomeClassWithConstant $t)[$t::C]: void {}
       *)
  | Hsoft of hint
  | Hrefinement of hint * refinement list
  (* The following constructors don't exist in the AST hint type *)
  | Hany
  | Herr
  | Hmixed
  | Hwildcard
  | Hnonnull
  | Habstr of string * hint list
  | Hvec_or_dict of hint option * hint
  | Hprim of (tprim[@transform.opaque])
  | Hthis
  | Hdynamic
  | Hnothing
  | Hunion of hint list
  | Hintersection of hint list
  | Hfun_context of string
  | Hvar of string

and refinement =
  | Rctx of sid * ctx_refinement
  | Rtype of sid * type_refinement

and type_refinement =
  | TRexact of hint
  | TRloose of type_refinement_bounds

and type_refinement_bounds = {
  tr_lower: hint list;
  tr_upper: hint list;
}

and ctx_refinement =
  | CRexact of hint
  | CRloose of ctx_refinement_bounds

and ctx_refinement_bounds = {
  cr_lower: hint option;
  cr_upper: hint option;
}

and shape_field_info = {
  sfi_optional: bool;
  sfi_hint: hint;
  sfi_name: Ast_defs.shape_field_name; [@transform.opaque]
}

and nast_shape_info = {
  nsi_allows_unknown_fields: bool;
  nsi_field_map: shape_field_info list;
}

and kvc_kind =
  | Map
  | ImmMap
  | Dict
[@@visitors.opaque] [@@transform.opaque]

and vc_kind =
  | Vector
  | ImmVector
  | Vec
  | Set
  | ImmSet
  | Keyset
[@@visitors.opaque] [@@transform.opaque]

and enum_ = {
  e_base: hint;
  e_constraint: hint option;
  e_includes: hint list;
}

and where_constraint_hint =
  hint * (Ast_defs.constraint_kind[@transform.opaque]) * hint
[@@deriving
  show { with_path = false },
    eq,
    hash,
    ord,
    map,
    transform ~restart:(`Disallow `Encode_as_result),
    visitors
      {
        variety = "iter";
        nude = true;
        visit_prefix = "on_";
        ancestors =
          ["Visitors_runtime.iter"; "Aast_defs_visitors_ancestors.iter"];
      },
    visitors
      {
        variety = "reduce";
        nude = true;
        visit_prefix = "on_";
        ancestors =
          ["Visitors_runtime.reduce"; "Aast_defs_visitors_ancestors.reduce"];
      },
    visitors
      {
        variety = "map";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["Visitors_runtime.map"; "Aast_defs_visitors_ancestors.map"];
      },
    visitors
      {
        variety = "endo";
        nude = true;
        visit_prefix = "on_";
        ancestors =
          ["Visitors_runtime.endo"; "Aast_defs_visitors_ancestors.endo"];
      }]

let string_of_tprim prim =
  match prim with
  | Tnull -> "null"
  | Tvoid -> "void"
  | Tint -> "int"
  | Tnum -> "num"
  | Tfloat -> "float"
  | Tstring -> "string"
  | Tarraykey -> "arraykey"
  | Tresource -> "resource"
  | Tnoreturn -> "noreturn"
  | Tbool -> "bool"

let string_of_visibility vis =
  match vis with
  | Private -> "private"
  | Public -> "public"
  | Protected -> "protected"
  | Internal -> "internal"

let is_wildcard_hint h =
  match h with
  | (_, Hwildcard) -> true
  | _ -> false
