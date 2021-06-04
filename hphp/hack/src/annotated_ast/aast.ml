(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Aast_defs

(* Ensure that doc comments are always on a separate line by
   requiring a lot of padding for inline doc comments. *)
[@@@ocamlformat "doc-comments-padding=80"]

(** Aast.program represents the top-level definitions in a Hack program.
 ex: Expression annotation type (when typechecking, the inferred type)
 fb: Function body tag (e.g. has naming occurred)
 en: Environment (tracking state inside functions and classes)
 hi: Hint annotation (when typechecking it will be the localized type hint or the
     inferred missing type if the hint is missing)
 *)
type ('ex, 'fb, 'en, 'hi) program = ('ex, 'fb, 'en, 'hi) def list
[@@deriving
  show { with_path = false },
    eq,
    visitors
      {
        variety = "iter";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["iter_defs"];
      },
    visitors
      {
        variety = "reduce";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["reduce_defs"];
      },
    visitors
      {
        variety = "map";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["map_defs"];
      },
    visitors
      {
        variety = "endo";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["endo_defs"];
      }]

and ('ex, 'fb, 'en, 'hi) stmt = pos * ('ex, 'fb, 'en, 'hi) stmt_

and ('ex, 'fb, 'en, 'hi) stmt_ =
  | Fallthrough
      (** Marker for a switch statement that falls through.

          // FALLTHROUGH *)
  | Expr of ('ex, 'fb, 'en, 'hi) expr
      (** Standalone expression.

          1 + 2; *)
  | Break
      (** Break inside a loop or switch statement.

          break; *)
  | Continue
      (** Continue inside a loop or switch statement.

          continue; *)
  | Throw of ('ex, 'fb, 'en, 'hi) expr
      (** Throw an exception.

          throw $foo; *)
  | Return of ('ex, 'fb, 'en, 'hi) expr option
      (** Return, with an optional value.

          return;
          return $foo; *)
  | Yield_break
      (** Yield break, terminating the current generator. This behaves like
          return; but is more explicit, and ensures the function is treated
          as a generator.

          yield break; *)
  | Awaitall of
      (* Temporaries assigned when running awaits. *)
      (lid option * ('ex, 'fb, 'en, 'hi) expr) list
      * (* Block assigning temporary to relevant locals. *)
      ('ex, 'fb, 'en, 'hi) block
      (** Concurrent block. All the await expressions are awaited at the
          same time, similar to genva().

          We store the desugared form. In the below example, the list is:
          [('__tmp$1', f()), (__tmp$2, g()), (None, h())]
          and the block assigns the temporary variables back to the locals.
          { $foo = __tmp$1; $bar = __tmp$2; }

          concurrent {
            $foo = await f();
            $bar = await g();
            await h();
          } *)
  | If of
      ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) block
      * ('ex, 'fb, 'en, 'hi) block
      (** If statement.

          if ($foo) { ... } else { ... } *)
  | Do of ('ex, 'fb, 'en, 'hi) block * ('ex, 'fb, 'en, 'hi) expr
      (** Do-while loop.

          do {
            bar();
          } while($foo) *)
  | While of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) block
      (** While loop.

          while ($foo) {
            bar();
          } *)
  | Using of ('ex, 'fb, 'en, 'hi) using_stmt
      (** Initialize a value that is automatically disposed of.

          using $foo = bar(); // disposed at the end of the function
          using ($foo = bar(), $baz = quux()) {} // disposed after the block *)
  | For of
      ('ex, 'fb, 'en, 'hi) expr list
      * ('ex, 'fb, 'en, 'hi) expr option
      * ('ex, 'fb, 'en, 'hi) expr list
      * ('ex, 'fb, 'en, 'hi) block
      (** For loop. The initializer and increment parts can include
          multiple comma-separated statements. The termination condition is
          optional.

          for ($i = 0; $i < 100; $i++) { ... }
          for ($x = 0, $y = 0; ; $x++, $y++) { ... } *)
  | Switch of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) case list
      (** Switch statement.

          switch ($foo) {
            case X:
              bar();
              break;
            default:
              baz();
              break;
          } *)
  | Foreach of
      ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) as_expr
      * ('ex, 'fb, 'en, 'hi) block
      (** For-each loop.

          foreach ($items as $item) { ... }
          foreach ($items as $key => value) { ... }
          foreach ($items await as $item) { ... } // AsyncIterator<_>
          foreach ($items await as $key => value) { ... } // AsyncKeyedIterator<_> *)
  | Try of
      ('ex, 'fb, 'en, 'hi) block
      * ('ex, 'fb, 'en, 'hi) catch list
      * ('ex, 'fb, 'en, 'hi) block
      (** Try statement, with catch blocks and a finally block.

          try {
            foo();
          } catch (SomeException $e) {
            bar();
          } finally {
            baz();
          } *)
  | Noop
      (** No-op, the empty statement.

          {}
          while (true) ;
          if ($foo) {} // the else is Noop here *)
  | Block of ('ex, 'fb, 'en, 'hi) block
      (** Block, a list of statements in curly braces.

          { $foo = 42; } *)
  | Markup of pstring
      (** The mode tag at the beginning of a file.
          TODO: this really belongs in def.

          <?hh *)
  | AssertEnv of env_annot * 'ex local_id_map
      (** Used in IFC to track type inference environments. Not user
          denotable. *)

and env_annot =
  | Join
  | Refinement

and ('ex, 'fb, 'en, 'hi) using_stmt = {
  us_is_block_scoped: bool;
  us_has_await: bool;
  us_exprs: pos * ('ex, 'fb, 'en, 'hi) expr list;
  us_block: ('ex, 'fb, 'en, 'hi) block;
}

and ('ex, 'fb, 'en, 'hi) as_expr =
  | As_v of ('ex, 'fb, 'en, 'hi) expr
  | As_kv of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr
  | Await_as_v of pos * ('ex, 'fb, 'en, 'hi) expr
  | Await_as_kv of pos * ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr

and ('ex, 'fb, 'en, 'hi) block = ('ex, 'fb, 'en, 'hi) stmt list

and ('ex, 'fb, 'en, 'hi) class_id = 'ex * ('ex, 'fb, 'en, 'hi) class_id_

(** Class ID, used in things like instantiation and static property access. *)
and ('ex, 'fb, 'en, 'hi) class_id_ =
  | CIparent
      (** The class ID of the parent of the lexically scoped class.

          In a trait, it is the parent class ID of the using class.

          parent::some_meth()
          parent::$prop = 1;
          new parent(); *)
  | CIself
      (** The class ID of the lexically scoped class.

          In a trait, it is the class ID of the using class.

          self::some_meth()
          self::$prop = 1;
          new self(); *)
  | CIstatic
      (** The class ID of the late static bound class.

          https://www.php.net/manual/en/language.oop5.late-static-bindings.php

          In a trait, it is the late static bound class ID of the using class.

          static::some_meth()
          static::$prop = 1;
          new static(); *)
  | CIexpr of ('ex, 'fb, 'en, 'hi) expr
      (** Dynamic class name.

          TODO: Syntactically this can only be an Lvar/This/Lplacehodller.
          We should use lid rather than expr.

          // Assume $d has type dynamic.
          $d::some_meth();
          $d::$prop = 1;
          new $d(); *)
  | CI of sid
      (** Explicit class name. This is the common case.

          Foop::some_meth()
          Foo::$prop = 1;
          new Foo(); *)

and ('ex, 'fb, 'en, 'hi) expr = 'ex * ('ex, 'fb, 'en, 'hi) expr_

and 'hi collection_targ =
  | CollectionTV of 'hi targ
  | CollectionTKV of 'hi targ * 'hi targ

and ('ex, 'fb, 'en, 'hi) function_ptr_id =
  | FP_id of sid
  | FP_class_const of ('ex, 'fb, 'en, 'hi) class_id * pstring

and ('ex, 'fb, 'en, 'hi) expression_tree = {
  et_hint: hint;
  et_splices: ('ex, 'fb, 'en, 'hi) block;
  et_virtualized_expr: ('ex, 'fb, 'en, 'hi) expr;
  et_runtime_expr: ('ex, 'fb, 'en, 'hi) expr;
}

and ('ex, 'fb, 'en, 'hi) expr_ =
  | Darray of
      ('hi targ * 'hi targ) option
      * (('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr) list
      (** darray literal.

          darray['x' => 0, 'y' => 1]
          darray<string, int>['x' => 0, 'y' => 1] *)
  | Varray of 'hi targ option * ('ex, 'fb, 'en, 'hi) expr list
      (** varray literal.

          varray['hello', 'world']
          varray<string>['hello', 'world'] *)
  | Shape of (Ast_defs.shape_field_name * ('ex, 'fb, 'en, 'hi) expr) list
      (** Shape literal.

          shape('x' => 1, 'y' => 2) *)
  | ValCollection of vc_kind * 'hi targ option * ('ex, 'fb, 'en, 'hi) expr list
      (** Collection literal for indexable structures.

           Vector {1, 2}
           ImmVector {}
           Set<string> {'foo', 'bar'}
           vec[1, 2]
           keyset[] *)
  | KeyValCollection of
      kvc_kind * ('hi targ * 'hi targ) option * ('ex, 'fb, 'en, 'hi) field list
      (** Collection literal for key-value structures.

          dict['x' => 1, 'y' => 2]
          Map<int, string> {}
          ImmMap {} *)
  | Null
      (** Null literal.

          null *)
  | This
      (** The local variable representing the current class instance.

          $this *)
  | True
      (** Boolean literal.

          true *)
  | False
      (** Boolean literal.

          false *)
  | Omitted
      (** The empty expression.

          list(, $y) = vec[1, 2] // Omitted is the first expression inside list() *)
  | Id of sid
      (** An identifier. Used for method names and global constants.

          SOME_CONST
          $x->foo() // id: "foo" *)
  | Lvar of lid
      (** Local variable.

          $foo *)
  | Dollardollar of lid
      (** The extra variable in a pipe expression.

          $$ *)
  | Clone of ('ex, 'fb, 'en, 'hi) expr
      (** Clone expression.

          clone $foo *)
  | Array_get of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr option
      (** Array indexing.

          $foo[]
          $foo[$bar] *)
  | Obj_get of
      ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) expr
      * og_null_flavor
      * (* is_prop_call *) bool
      (** Instance property or method access.  is_prop_call is always
          false, except when inside a call is accessing a property.

          $foo->bar // (Obj_get false) property access
          $foo->bar() // (Call (Obj_get false)) method call
          ($foo->bar)() // (Call (Obj_get true)) call lambda stored in property
          $foo?->bar // nullsafe access *)
  | Class_get of
      ('ex, 'fb, 'en, 'hi) class_id
      * ('ex, 'fb, 'en, 'hi) class_get_expr
      * (* is_prop_call *) bool
      (** Static property access.

          Foo::$bar
          $some_classname::$bar
          Foo::${$bar} // only in partial mode *)
  | Class_const of ('ex, 'fb, 'en, 'hi) class_id * pstring
      (** Class constant or static method call. As a standalone expression,
          this is a class constant. Inside a Call node, this is a static
          method call.

          This is not ambiguous, because constants are not allowed to
          contain functions.

          Foo::some_const // Class_const
          Foo::someStaticMeth() // Call (Class_const)

          This syntax is used for both static and instance methods when
          calling the implementation on the superclass.

          parent::someStaticMeth()
          parent::someInstanceMeth() *)
  | Call of
      (* function *)
      ('ex, 'fb, 'en, 'hi) expr
      * (* explicit type annotations *)
      'hi targ list
      * (* positional args *)
      ('ex, 'fb, 'en, 'hi) expr list
      * (* unpacked arg *)
      ('ex, 'fb, 'en, 'hi) expr option
      (** Function or method call.

          foo()
          $x()
          foo<int>(1, 2, ...$rest)
          $x->foo()

          async { return 1; }
          // lowered to:
          (async () ==> { return 1; })() *)
  | FunctionPointer of ('ex, 'fb, 'en, 'hi) function_ptr_id * 'hi targ list
      (** A reference to a function or method.

          foo_fun<>
          FooCls::meth<int> *)
  | Int of string
      (** Integer literal.

          42
          0123 // octal
          0xBEEF // hexadecimal
          0b11111111 // binary *)
  | Float of string
      (** Float literal.

          1.0
          1.2e3
          7E-10 *)
  | String of byte_string
      (** String literal.

          "foo"
          'foo'

          <<<DOC
          foo
          DOC

          <<<'DOC'
          foo
          DOC *)
  | String2 of ('ex, 'fb, 'en, 'hi) expr list
      (** Interpolated string literal.

          "hello $foo $bar"

          <<<DOC
          hello $foo $bar
          DOC *)
  | PrefixedString of string * ('ex, 'fb, 'en, 'hi) expr
      (** Prefixed string literal. Only used for regular expressions.

          re"foo" *)
  | Yield of ('ex, 'fb, 'en, 'hi) afield
      (** Yield expression. The enclosing function should have an Iterator
          return type.

          yield $foo // enclosing function returns an Iterator
          yield $foo => $bar // enclosing function returns a KeyedIterator *)
  | Await of ('ex, 'fb, 'en, 'hi) expr
      (** Await expression.

          await $foo *)
  | ReadonlyExpr of ('ex, 'fb, 'en, 'hi) expr
      (** Readonly expression.

          readonly $foo *)
  | Tuple of ('ex, 'fb, 'en, 'hi) expr list
      (** Tuple expression.

          tuple("a", 1, $foo) *)
  | List of ('ex, 'fb, 'en, 'hi) expr list
      (** List expression, only used in destructuring. Allows any arbitrary
          lvalue as a subexpression. May also nest.

          list($x, $y) = vec[1, 2];
          list(, $y) = vec[1, 2]; // skipping items
          list(list($x)) = vec[vec[1]]; // nesting
          list($v[0], $x[], $y->foo) = $blah; *)
  | Cast of hint * ('ex, 'fb, 'en, 'hi) expr
      (** Cast expression, converting a value to a different type. Only
          primitive types are supported in the hint position.

          (int)$foo
          (string)$foo *)
  | Unop of Ast_defs.uop * ('ex, 'fb, 'en, 'hi) expr
      (** Unary operator.

          !$foo
          -$foo
          +$foo *)
  | Binop of
      Ast_defs.bop * ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr
      (** Binary operator.

          $foo + $bar *)
  | Pipe of lid * ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr
      (** Pipe expression. The lid is the ID of the $$ that is implicitly
          declared by this pipe.

          See also Dollardollar.

          $foo |> bar() // equivalent: bar($foo)
          $foo |> bar(1, $$) // equivalent: bar(1, $foo) *)
  | Eif of
      ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) expr option
      * ('ex, 'fb, 'en, 'hi) expr
      (** Ternary operator, or elvis operator.

          $foo ? $bar : $baz // ternary
          $foo ?: $baz // elvis *)
  | Is of ('ex, 'fb, 'en, 'hi) expr * hint
      (** Is operator.

          $foo is SomeType *)
  | As of ('ex, 'fb, 'en, 'hi) expr * hint * (* is nullable *) bool
      (** As operator.

          $foo as int
          $foo ?as int *)
  | New of
      ('ex, 'fb, 'en, 'hi) class_id
      * 'hi targ list
      * ('ex, 'fb, 'en, 'hi) expr list
      * ('ex, 'fb, 'en, 'hi) expr option
      * (* constructor *)
      'ex
      (** Instantiation.

          new Foo(1, 2);
          new Foo<int, T>();
          new Foo('blah', ...$rest); *)
  | Record of sid * (('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr) list
      (** Record literal.

          MyRecord['x' => $foo, 'y' => $bar] *)
  | Efun of ('ex, 'fb, 'en, 'hi) fun_ * lid list
      (** PHP-style lambda. Does not capture variables unless explicitly
          specified.

          Mnemonic: 'expanded lambda', since we can desugar Lfun to Efun.

          function($x) { return $x; }
          function(int $x): int { return $x; }
          function($x) use ($y) { return $y; }
          function($x): int use ($y, $z) { return $x + $y + $z; } *)
  | Lfun of ('ex, 'fb, 'en, 'hi) fun_ * lid list
      (** Hack lambda. Captures variables automatically.

          $x ==> $x
          (int $x): int ==> $x + $other
          ($x, $y) ==> { return $x + $y; } *)
  | Xml of
      sid
      * ('ex, 'fb, 'en, 'hi) xhp_attribute list
      * ('ex, 'fb, 'en, 'hi) expr list
      (** XHP expression. May contain interpolated expressions.

          <foo x="hello" y={$foo}>hello {$bar}</foo> *)
  | Callconv of Ast_defs.param_kind * ('ex, 'fb, 'en, 'hi) expr
      (** Explicit calling convention, used for inout. Inout supports any lvalue.

          TODO: This could be a flag on parameters in Call.

          foo(inout $x[0]) *)
  | Import of import_flavor * ('ex, 'fb, 'en, 'hi) expr
      (** Include or require expression.

          require('foo.php')
          require_once('foo.php')
          include('foo.php')
          include_once('foo.php') *)
  | Collection of
      sid * 'hi collection_targ option * ('ex, 'fb, 'en, 'hi) afield list
      (** Collection literal.

          TODO: T38184446 this is redundant with ValCollection/KeyValCollection.

          Vector {} *)
  | ExpressionTree of ('ex, 'fb, 'en, 'hi) expression_tree
      (** Expression tree literal. Expression trees are not evaluated at
          runtime, but desugared to an expression representing the code.

          Foo`1 + bar()`
          Foo`$x ==> $x * ${$value}` // splicing $value *)
  | Lplaceholder of pos
      (** Placeholder local variable.

          $_ *)
  | Fun_id of sid
      (** Global function reference.

          fun('foo') *)
  | Method_id of ('ex, 'fb, 'en, 'hi) expr * pstring
      (** Instance method reference on a specific instance.

          TODO: This is only created in naming, and ought to happen in
          lowering or be removed. The emitter just sees a normal Call.

          inst_meth($f, 'some_meth') // equivalent: $f->some_meth<> *)
  | Method_caller of sid * pstring
      (** Instance method reference that can be called with an instance.

          meth_caller(FooClass::class, 'some_meth')
          meth_caller('FooClass', 'some_meth')

          These examples are equivalent to:

          (FooClass $f, ...$args) ==> $f->some_meth(...$args) *)
  | Smethod_id of ('ex, 'fb, 'en, 'hi) class_id * pstring
      (** Static method reference.

          class_meth('FooClass', 'some_static_meth')
          // equivalent: FooClass::some_static_meth<> *)
  | Pair of
      ('hi targ * 'hi targ) option
      * ('ex, 'fb, 'en, 'hi) expr
      * ('ex, 'fb, 'en, 'hi) expr
      (** Pair literal.

          Pair {$foo, $bar} *)
  | ET_Splice of ('ex, 'fb, 'en, 'hi) expr
      (** Expression tree splice expression. Only valid inside an
          expression tree literal (backticks).

          ${$foo} *)
  | EnumClassLabel of sid option * string
      (** Label used for enum classes.

          enum_name#label_name or #label_name *)
  | Any
      (** Placeholder for expressions that aren't understood by parts of
          the toolchain.

          TODO: Remove. *)
  | Hole of ('ex, 'fb, 'en, 'hi) expr * 'hi * 'hi * hole_source
      (** Annotation used to record failure in subtyping or coercion of an
          expression and calls to [unsafe_cast] or [enforced_cast].

          The [hole_source] indicates whether this came from an
          explicit call to [unsafe_cast] or [enforced_cast] or was
          generated during typing.

          Given a call to [unsafe_cast]:
          ```
          function f(int $x): void { /* ... */ }

          function g(float $x): void {
             f(unsafe_cast<float,int>($x));
          }
          ```
          After typing, this is represented by the following TAST fragment
          ```
          Call
            ( ( (..., function(int $x): void), Id (..., "\f"))
            , []
            , [ ( (..., int)
                , Hole
                    ( ((..., float), Lvar (..., $x))
                    , float
                    , int
                    , UnsafeCast
                    )
                )
              ]
            , None
            )
          ```
      *)

and ('ex, 'fb, 'en, 'hi) class_get_expr =
  | CGstring of pstring
  | CGexpr of ('ex, 'fb, 'en, 'hi) expr

and ('ex, 'fb, 'en, 'hi) case =
  | Default of pos * ('ex, 'fb, 'en, 'hi) block
  | Case of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) block

and ('ex, 'fb, 'en, 'hi) catch = sid * lid * ('ex, 'fb, 'en, 'hi) block

and ('ex, 'fb, 'en, 'hi) field =
  ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr

and ('ex, 'fb, 'en, 'hi) afield =
  | AFvalue of ('ex, 'fb, 'en, 'hi) expr
  | AFkvalue of ('ex, 'fb, 'en, 'hi) expr * ('ex, 'fb, 'en, 'hi) expr

and ('ex, 'fb, 'en, 'hi) xhp_simple = {
  xs_name: pstring;
  xs_type: 'hi;
  xs_expr: ('ex, 'fb, 'en, 'hi) expr;
}

and ('ex, 'fb, 'en, 'hi) xhp_attribute =
  | Xhp_simple of ('ex, 'fb, 'en, 'hi) xhp_simple
  | Xhp_spread of ('ex, 'fb, 'en, 'hi) expr

and is_variadic = bool

and ('ex, 'fb, 'en, 'hi) fun_param = {
  param_annotation: 'ex;
  param_type_hint: 'hi type_hint;
  param_is_variadic: is_variadic;
  param_pos: pos;
  param_name: string;
  param_expr: ('ex, 'fb, 'en, 'hi) expr option;
  param_readonly: Ast_defs.readonly_kind option;
  param_callconv: Ast_defs.param_kind option;
  param_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  param_visibility: visibility option;
}

(** Does this function/method take a variable number of arguments? *)
and ('ex, 'fb, 'en, 'hi) fun_variadicity =
  | FVvariadicArg of ('ex, 'fb, 'en, 'hi) fun_param
      (** Named variadic argument.

      function foo(int ...$args): void {} *)
  | FVellipsis of pos
      (** Unnamed variaidic argument. Partial mode only.

          function foo(...): void {} *)
  | FVnonVariadic
      (** Function is not variadic, takes an exact number of arguments. *)

and ('ex, 'fb, 'en, 'hi) fun_ = {
  f_span: pos;
  f_readonly_this: Ast_defs.readonly_kind option;
  f_annotation: 'en;
  f_readonly_ret: Ast_defs.readonly_kind option;
  (* Whether the return value is readonly *)
  f_ret: 'hi type_hint;
  f_name: sid;
  f_tparams: ('ex, 'fb, 'en, 'hi) tparam list;
  f_where_constraints: where_constraint_hint list;
  f_variadic: ('ex, 'fb, 'en, 'hi) fun_variadicity;
  f_params: ('ex, 'fb, 'en, 'hi) fun_param list;
  f_ctxs: contexts option;
  f_unsafe_ctxs: contexts option;
  f_body: ('ex, 'fb, 'en, 'hi) func_body;
  f_fun_kind: Ast_defs.fun_kind;
  f_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  f_external: bool;
      (** true if this declaration has no body because it is an
                         external function declaration (e.g. from an HHI file)*)
  f_doc_comment: doc_comment option;
}

(**
 * Naming has two phases and the annotation helps to indicate the phase.
 * In the first pass, it will perform naming on everything except for function
 * and method bodies and collect information needed. Then, another round of
 * naming is performed where function bodies are named. Thus, naming will
 * have named and unnamed variants of the annotation.
 * See BodyNamingAnnotation in nast.ml and the comment in naming.ml
 *)
and ('ex, 'fb, 'en, 'hi) func_body = {
  fb_ast: ('ex, 'fb, 'en, 'hi) block;
  fb_annotation: 'fb;
}

(** A type annotation is two things:
  - the localized hint, or if the hint is missing, the inferred type
  - The typehint associated to this expression if it exists *)
and 'hi type_hint = 'hi * type_hint_

(** Explicit type argument to function, constructor, or collection literal.
 * 'hi = unit in NAST
 * 'hi = Typing_defs.(locl ty) in TAST,
 * and is used to record inferred type arguments, with wildcard hint.
 *)
and 'hi targ = 'hi * hint

and type_hint_ = hint option

and ('ex, 'fb, 'en, 'hi) user_attribute = {
  ua_name: sid;
  ua_params: ('ex, 'fb, 'en, 'hi) expr list;
      (** user attributes are restricted to scalar values *)
}

and ('ex, 'fb, 'en, 'hi) file_attribute = {
  fa_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  fa_namespace: nsenv;
}

and ('ex, 'fb, 'en, 'hi) tparam = {
  tp_variance: Ast_defs.variance;
  tp_name: sid;
  tp_parameters: ('ex, 'fb, 'en, 'hi) tparam list;
  tp_constraints: (Ast_defs.constraint_kind * hint) list;
  tp_reified: reify_kind;
  tp_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
}

and use_as_alias = sid option * pstring * sid option * use_as_visibility list

and insteadof_alias = sid * pstring * sid list

and is_extends = bool

and emit_id =
  (* For globally defined type, the ID used in the .main function. *)
  | Emit_id of int
  (* Closures are hoisted to classes, but they don't get an entry in .main. *)
  | Anonymous

and ('ex, 'fb, 'en, 'hi) class_ = {
  c_span: pos;
  c_annotation: 'en;
  c_mode: FileInfo.mode; [@visitors.opaque]
  c_final: bool;
  c_is_xhp: bool;
  c_has_xhp_keyword: bool;
  c_kind: Ast_defs.class_kind;
  c_name: sid;
  c_tparams: ('ex, 'fb, 'en, 'hi) tparam list;
      (** The type parameters of a class A<T> (T is the parameter) *)
  c_extends: class_hint list;
  c_uses: trait_hint list;
  c_use_as_alias: use_as_alias list;
      (** PHP feature not supported in hack but required
          because we have runtime support. *)
  c_insteadof_alias: insteadof_alias list;
      (** PHP feature not supported in hack but required
          because we have runtime support. *)
  c_xhp_attr_uses: xhp_attr_hint list;
  c_xhp_category: (pos * pstring list) option;
  c_reqs: (class_hint * is_extends) list;
  c_implements: class_hint list;
  c_support_dynamic_type: bool;
  c_where_constraints: where_constraint_hint list;
  c_consts: ('ex, 'fb, 'en, 'hi) class_const list;
  c_typeconsts: ('ex, 'fb, 'en, 'hi) class_typeconst_def list;
  c_vars: ('ex, 'fb, 'en, 'hi) class_var list;
  c_methods: ('ex, 'fb, 'en, 'hi) method_ list;
  c_attributes: ('ex, 'fb, 'en, 'hi) class_attr list;
  c_xhp_children: (pos * xhp_child) list;
  c_xhp_attrs: ('ex, 'fb, 'en, 'hi) xhp_attr list;
  c_namespace: nsenv;
  c_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  c_file_attributes: ('ex, 'fb, 'en, 'hi) file_attribute list;
  c_enum: enum_ option;
  c_doc_comment: doc_comment option;
  c_emit_id: emit_id option;
}

and class_hint = hint

and trait_hint = hint

and xhp_attr_hint = hint

and xhp_attr_tag =
  | Required
  | LateInit

and ('ex, 'fb, 'en, 'hi) xhp_attr =
  'hi type_hint
  * ('ex, 'fb, 'en, 'hi) class_var
  * xhp_attr_tag option
  * (pos * ('ex, 'fb, 'en, 'hi) expr list) option

and ('ex, 'fb, 'en, 'hi) class_attr =
  | CA_name of sid
  | CA_field of ('ex, 'fb, 'en, 'hi) ca_field

and ('ex, 'fb, 'en, 'hi) ca_field = {
  ca_type: ca_type;
  ca_id: sid;
  ca_value: ('ex, 'fb, 'en, 'hi) expr option;
  ca_required: bool;
}

and ca_type =
  | CA_hint of hint
  | CA_enum of string list

and ('ex, 'fb, 'en, 'hi) class_const = {
  cc_type: hint option;
  cc_id: sid;
  cc_expr: ('ex, 'fb, 'en, 'hi) expr option;
      (** expr = None indicates an abstract const *)
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
  c_atc_default: hint option;
}

and class_concrete_typeconst = { c_tc_type: hint }

(* A partially abstract type constant always has a constraint *
 * and always has a value. *)
and class_partially_abstract_typeconst = {
  c_patc_constraint: hint;
  c_patc_type: hint;
}

and class_typeconst =
  | TCAbstract of class_abstract_typeconst
  | TCConcrete of class_concrete_typeconst
  | TCPartiallyAbstract of class_partially_abstract_typeconst

and ('ex, 'fb, 'en, 'hi) class_typeconst_def = {
  c_tconst_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  c_tconst_name: sid;
  c_tconst_kind: class_typeconst;
  c_tconst_span: pos;
  c_tconst_doc_comment: doc_comment option;
  c_tconst_is_ctx: bool;
}

and xhp_attr_info = {
  xai_tag: xhp_attr_tag option;
  xai_enum_values: Ast_defs.xhp_enum_value list;
}

and ('ex, 'fb, 'en, 'hi) class_var = {
  cv_final: bool;
  cv_xhp_attr: xhp_attr_info option;
  cv_abstract: bool;
  cv_readonly: bool;
  cv_visibility: visibility;
  cv_type: 'hi type_hint;
  cv_id: sid;
  cv_expr: ('ex, 'fb, 'en, 'hi) expr option;
  cv_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  cv_doc_comment: doc_comment option;
  cv_is_promoted_variadic: bool;
  cv_is_static: bool;
  cv_span: pos;
}

and ('ex, 'fb, 'en, 'hi) method_ = {
  m_span: pos;
  m_annotation: 'en;
  m_final: bool;
  m_abstract: bool;
  m_static: bool;
  m_readonly_this: bool;
  m_visibility: visibility;
  m_name: sid;
  m_tparams: ('ex, 'fb, 'en, 'hi) tparam list;
  m_where_constraints: where_constraint_hint list;
  m_variadic: ('ex, 'fb, 'en, 'hi) fun_variadicity;
  m_params: ('ex, 'fb, 'en, 'hi) fun_param list;
  m_ctxs: contexts option;
  m_unsafe_ctxs: contexts option;
  m_body: ('ex, 'fb, 'en, 'hi) func_body;
  m_fun_kind: Ast_defs.fun_kind;
  m_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  m_readonly_ret: Ast_defs.readonly_kind option;
  m_ret: 'hi type_hint;
  m_external: bool;
      (** true if this declaration has no body because it is an external method
          declaration (e.g. from an HHI file) *)
  m_doc_comment: doc_comment option;
}

and nsenv = (Namespace_env.env[@visitors.opaque])

and ('ex, 'fb, 'en, 'hi) typedef = {
  t_annotation: 'en;
  t_name: sid;
  t_tparams: ('ex, 'fb, 'en, 'hi) tparam list;
  t_constraint: hint option;
  t_kind: hint;
  t_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  t_mode: FileInfo.mode; [@visitors.opaque]
  t_vis: typedef_visibility;
  t_namespace: nsenv;
  t_span: pos;
  t_emit_id: emit_id option;
}

and ('ex, 'fb, 'en, 'hi) gconst = {
  cst_annotation: 'en;
  cst_mode: FileInfo.mode; [@visitors.opaque]
  cst_name: sid;
  cst_type: hint option;
  cst_value: ('ex, 'fb, 'en, 'hi) expr;
  cst_namespace: nsenv;
  cst_span: pos;
  cst_emit_id: emit_id option;
}

and ('ex, 'fb, 'en, 'hi) record_def = {
  rd_annotation: 'en;
  rd_name: sid;
  rd_extends: record_hint option;
  rd_abstract: bool;
  rd_fields: (sid * hint * ('ex, 'fb, 'en, 'hi) expr option) list;
  rd_user_attributes: ('ex, 'fb, 'en, 'hi) user_attribute list;
  rd_namespace: nsenv;
  rd_span: pos;
  rd_doc_comment: doc_comment option;
  rd_emit_id: emit_id option;
}

and record_hint = hint

and ('ex, 'fb, 'en, 'hi) fun_def = {
  fd_namespace: nsenv;
  fd_file_attributes: ('ex, 'fb, 'en, 'hi) file_attribute list;
  fd_mode: FileInfo.mode; [@visitors.opaque]
  fd_fun: ('ex, 'fb, 'en, 'hi) fun_;
}

and ('ex, 'fb, 'en, 'hi) def =
  | Fun of ('ex, 'fb, 'en, 'hi) fun_def
  | Class of ('ex, 'fb, 'en, 'hi) class_
  | RecordDef of ('ex, 'fb, 'en, 'hi) record_def
  | Stmt of ('ex, 'fb, 'en, 'hi) stmt
  | Typedef of ('ex, 'fb, 'en, 'hi) typedef
  | Constant of ('ex, 'fb, 'en, 'hi) gconst
  | Namespace of sid * ('ex, 'fb, 'en, 'hi) program
  | NamespaceUse of (ns_kind * sid * sid) list
  | SetNamespaceEnv of nsenv
  | FileAttributes of ('ex, 'fb, 'en, 'hi) file_attribute

and ns_kind =
  | NSNamespace
  | NSClass
  | NSClassAndNamespace
  | NSFun
  | NSConst

and hole_source =
  | Typing
  | UnsafeCast
  | EnforcedCast

and doc_comment = (Doc_comment.t[@visitors.opaque])

let is_erased = function
  | Erased -> true
  | SoftReified
  | Reified ->
    false

let is_reified = function
  | Reified -> true
  | Erased
  | SoftReified ->
    false

let is_soft_reified = function
  | SoftReified -> true
  | Erased
  | Reified ->
    false

(* Splits the methods on a class into the constructor, statics, dynamics *)

(**
 * Methods, properties, and requirements are order dependent in bytecode
 * emission, which is observable in user code via `ReflectionClass`.
 *)
let split_methods class_ =
  let (constr, statics, res) =
    List.fold_left
      (fun (constr, statics, rest) m ->
        if snd m.m_name = "__construct" then
          (Some m, statics, rest)
        else if m.m_static then
          (constr, m :: statics, rest)
        else
          (constr, statics, m :: rest))
      (None, [], [])
      class_.c_methods
  in
  (constr, List.rev statics, List.rev res)

(* Splits class properties into statics, dynamics *)
let split_vars class_ =
  let (statics, res) =
    List.fold_left
      (fun (statics, rest) v ->
        if v.cv_is_static then
          (v :: statics, rest)
        else
          (statics, v :: rest))
      ([], [])
      class_.c_vars
  in
  (List.rev statics, List.rev res)

(* Splits `require`s into extends, implements *)
let split_reqs class_ =
  let (extends, implements) =
    List.fold_left
      (fun (extends, implements) (h, is_extends) ->
        if is_extends then
          (h :: extends, implements)
        else
          (extends, h :: implements))
      ([], [])
      class_.c_reqs
  in
  (List.rev extends, List.rev implements)

type break_continue_level =
  | Level_ok of int option
  | Level_non_literal
  | Level_non_positive

let get_break_continue_level level_opt =
  match level_opt with
  | (_, Int s) ->
    let i = int_of_string s in
    if i <= 0 then
      Level_non_positive
    else
      Level_ok (Some i)
  | _ -> Level_non_literal
  | exception _ -> Level_non_literal

(* extract the hint from a type annotation *)
let hint_of_type_hint : 'hi. 'hi type_hint -> type_hint_ = snd

(* extract the type from a type annotation *)
let type_of_type_hint : 'hi. 'hi type_hint -> 'hi = fst

(* map a function over the second part of a type annotation *)
let type_hint_option_map ~f ta =
  let mapped_hint =
    match hint_of_type_hint ta with
    | Some hint -> Some (f hint)
    | None -> None
  in
  (type_of_type_hint ta, mapped_hint)

(* helper function to access the list of enums included by an enum *)
let enum_includes_map ?(default = []) ~f includes =
  match includes with
  | None -> default
  | Some includes -> f includes

let is_enum_class c =
  match (c.c_kind, c.c_enum) with
  | (Ast_defs.Cenum, Some enum_) -> enum_.e_enum_class
  | (_, _) -> false
