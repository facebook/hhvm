Expression trees are an experimental Hack feature that allow you write domain specific languages (DSLs) with Hack syntax.

You can write expressions using normal Hack syntax, and all the normal Hack tools work: syntax highlighting, type checking, and code completion.

Unlike normal Hack code, **expression tree code is not executed**.

```hack no-extract
$e = MyDsl`foo() + 1`;
```

## What is executed at runtime?

Hack converts an expression tree to a series of method calls on a visitor.

```hack no-extract
// You write code like this:
$e = MyDsl`1 + 2`;

// Hack runs the following "desugared" code:
$e =
  MyDsl::makeTree<MyDslInt>(
    ($v) ==> $v->visitBinop(
      $v->visitInt(1),
      '__plus'
      $v->visitInt(2))
  );
```

The visitor can also define the types associated with literals and operators. This enables Hack to typecheck the DSL expression, even if `1` doesn't represent a Hack `int`.

```hack no-extract
// The type checker will also check that this "virtualized" expression is correct,
// even though this virtualized code isn't executed.
(MyDsl::intType())->__plus(MyDsl::intType());
```

## Why?

DSLs without expression trees are verbose and unchecked.

```hack no-extract
// No checking or highlighting at all.
takes_dsl_string("foo()");

// Verbose, limited type checking, no support for loops or variables in the DSL.
takes_dsl_object(new DslFunctionCall("foo"));
```

## When is this useful?

DSLs are useful when you're generating code for scripting languages, such as JavaScript in the browser or stored procedures in a database.
