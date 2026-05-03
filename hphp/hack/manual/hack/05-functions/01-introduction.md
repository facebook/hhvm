# Introduction

The `function` keyword defines a global function.

```hack
function add_one(int $x): int {
  return $x + 1;
}
```

The `function` keyword can also be used to define [methods](/hack/classes/methods).

## Default Parameters

Hack supports default values for parameters.

```hack
function add_value(int $x, int $y = 1): int {
  return $x + $y;
}
```

This function can take one or two arguments. `add_value(3)` returns 4.

Required parameters must come before optional parameters, so the
following code is invalid:

```hack error
function add_value_bad(int $x = 1, int $y): int {
  return $x + $y;
}
```

## Variadic Functions

You can use `...` to define a function that takes a variable number of
arguments.

```hack file:sumints.hack
function sum_ints(int $val, int ...$vals): int {
  $result = $val;

  foreach ($vals as $v) {
    $result += $v;
  }
  return $result;
}
```

This function requires at least one argument, but has no maximum
number of arguments.

```hack file:sumints.hack
// Passing positional arguments.
sum_ints(1, 2, 3);

// You can also pass a collection into a variadic parameter.
$args = vec[1, 2, 3];
sum_ints(0, ...$args);
```

## Unpacking Tuples as Arguments

You can use `...` to unpack a tuple as positional arguments to a function call. The type checker matches each tuple element against the corresponding parameter type.

```hack
function add(int $x, int $y): int {
  return $x + $y;
}

function call_with_tuple(): void {
  $args = tuple(1, 2);
  $result = add(...$args);
}
```

Positional arguments can precede the splat, but no arguments can follow it — even when the splatted tuple has a known, fixed length.

```hack
function greet(string $greeting, string $name, int $times): void {
  for ($i = 0; $i < $times; $i++) {
    echo $greeting.' '.$name."\n";
  }
}

function call_mixed(): void {
  $rest = tuple('Alice', 3);
  greet('Hello', ...$rest);
}
```

The type checker enforces that the tuple has the right number of elements and that each element's type matches the corresponding parameter. The splatted value must be a tuple type — `vec` and other container types cannot be used this way. When the tuple type itself has optional or variadic elements, the splat respects those — for details on tuple type syntax, see [tuples](/hack/built-in-types/tuples).

For spreading tuple types into function type signatures, see [type splat](26-type-splat.md).

## Function Types

Functions are values in Hack, so they can be passed as arguments too.

```hack
function apply_func(int $v, (function(int): int) $f): int {
  return $f($v);
}

function usage_example(): void {
  $x = apply_func(0, $x ==> $x + 1);
}
```

Variadic functions can also be passed as arguments.

```hack
function takes_variadic_fun((function(int...): void) $f): void {
  $f(1, 2, 3);

  $args = vec[1, 2, 3];
  $f(0, ...$args);
}
```

Finally, functions taking optional parameters can be passed as arguments.

```hack
function with_default(int $x, int $y = 0):void {
}
function takes_unary_or_binary_fun((function(int,optional int):void) $f):void {
  $f(1);
  $f(1,2);
}
function demo():void {
  takes_unary_or_binary_fun(with_default<>);
}
```
