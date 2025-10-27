# Constants

A class can contain definitions for named constants.

Because a class constant belongs to the class as a whole, it is implicitly `static`. For example:

```hack
class Automobile {
  const DEFAULT_COLOR = "white";
  // ...
}

<<__EntryPoint>>
function main(): void {
  $col = Automobile::DEFAULT_COLOR; // or: $col = self::DEFAULT_COLOR;
  echo "\$col = $col\n";
}
```

## Visibility
Class constants are always public, and can not be explicitly declared as `public`, `protected`, or `private`.

## Selection
Inside a parent class, use `self::foo` to access a named constant `foo`. Outside a parent class, a class constant's name must be fully qualified with the class and constant name (e.g. `Bar::foo`). For more information, see [scope-resolution operator, `::`](/hack/expressions-and-operators/scope-resolution).

## Type Inference
If a class constant's type is omitted, it can be inferred. For example:
* the inferred type of `const DEFAULT_COLOR = "white"` is `string`,
* the inferred type of `const DEFAULT_VALUE = 42` is `int`.
* the inferred type of `const DEFAULT_FOODS = vec["apple", "orange", "banana"]` is `vec`.

## Limitations
Constants can not be assigned to legacy container types like `Vector`, `Map`, `Set`, et al., and closures.

Instead, create constants with equivalent types like `array`, `vec`, `dict`, and `set`. When using these types, all subinitializers must resolve to constant expressions.
