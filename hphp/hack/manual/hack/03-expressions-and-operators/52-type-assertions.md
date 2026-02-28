# Type Assertions

Hack provides the `is` and `as` operators for inspecting types at
runtime. To convert primitive types, see [casting](/hack/expressions-and-operators/casting).

The type checker also understands `is` and `as`, so it will infer
precise types.

## Checking Types with `is`

The `is` operator checks whether a value has the type specified, and
returns a boolean result.

```hack
1 is int;        // true
'foo' is int;    // false

1 is num;        // true
1.5 is num;      // true
'foo' is num;    // false
```

The type checker understands `is` and [refines
values](/hack/types/type-refinement) inside conditionals or after
`invariant` calls.

```hack no-extract
function transport(Vehicle $v): void {
  if ($v is Car) {
    $v->drive();
  } else if ($v is Plane) {
    $v->fly();
  } else {
    invariant($v is Boat, "Expected a boat");
    $v->sail();
  }
}
```

A common pattern with `is` refinement is to use `nonnull` rather than
an explicit type.

``` Hack no-extract
function transport(?Car $c): void {
  if ($c is nonnull) {
    // Infers that $c is Car, but saves us
    // repeating the name of the type.
    $c->drive();
  }
}
```

### Enums

For enums, `is` also checks that the value is valid.

```hack
enum MyEnum: int {
  FOO = 1;
}

function demo(): void {
  1 is MyEnum;       // true
  42 is MyEnum;      // false
  'foo' is MyEnum;   // false
}
```

### Generics

Since `is` provides a runtime check, it cannot be used with erased
generics. For generic types, you must use `_` placeholders for type
parameters.

```
$v = vec[1, 2, 3];

// We can't use `is vec<int>` here.
$v is vec<_>; // true
```

If you need to check inner types at runtime, consider using reified
generics instead.

### Tuples

For tuples and shapes, `is` validates the size and recursively validates every field in the value.

```hack
$x = tuple(1, 2.0, null);
$x is (int, float, ?bool); // true

$y = shape('foo' => 1);
$y is shape('foo' => int); // true
```

### Aliases

`is` also works with type aliases and type constants, by testing
against the underlying runtime type.

``` Hack
type myint = int;

function demo(): void {
  1 is myint; // true
}
```

## Enforcing Types with `as` and `?as`

`as` performs the same checks as `is`.

However, it throws `TypeAssertionException` if the value has a
different type. The type checker understands that the value must have
the type specified afterwards, so it
[refines](/hack/types/type-refinement) the value.

```hack
1 as int;        // 1
'foo' as int;    // TypeAssertionException
```

`as` enables you to narrow a type.

```hack no-extract
// Normally you'd want to make transport take a Vehicle
// directly, so you can check when you call the function.
function transport(mixed $m): void {
  // Exception if not a Vehicle.
  $v = $m as Vehicle;

  if ($v is Car) {
    $v->drive();
  } else {
    // Exception if $v is not a Boat.
    $v as Boat;
    $v->sail();
  }
}
```

Hack also provides `?as`, which returns `null` if the type does not match.


```hack
1 ?as int;        // 1
'foo' ?as int;    // null
```

Note that `as` can also be used in type signatures when using
generics.

## Legacy Type Predicates

Hack also provides type predicate functions `is_int`, `is_bool` and so
on. You should use `is` instead.
