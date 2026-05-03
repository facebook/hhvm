# Nullsafe Pipe

The nullsafe pipe operator, `|?>`, is a variant of the [pipe operator](93-pipe.md) designed for nullable expressions. It evaluates the left-hand expression and, if the result is `null`, short-circuits the entire expression to `null` without evaluating the right-hand side. If the result is non-null, it binds the value to `$$` with nullability stripped and evaluates the right-hand expression.

The result type of `e1 |?> e2` is always `?ty2` — even if `e2` would return a non-nullable type on its own, the overall expression can still be `null` because of the short-circuit.

## Basic Usage

```hack
function foo(bool $coinflip): ?string {
  return $coinflip ? "foo" : null;
}

function bar(string $prefix): string {
  return $prefix."bar";
}

function example(bool $coinflip): void {
  // $$ is refined to string (not ?string), so bar() accepts it.
  // The result is ?string because |?> may short-circuit to null.
  $result = foo($coinflip) |?> bar($$);
}
```

Without the nullsafe pipe, you would need an explicit null check:

```hack no-extract
function example_without(bool $coinflip): void {
  $tmp = foo($coinflip);
  $result = $tmp is nonnull ? bar($tmp) : null;
}
```
