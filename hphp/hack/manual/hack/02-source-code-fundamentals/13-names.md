# Names

Names are used to label variables, constants, functions, and user-defined types, among other things. A name *must* begin
with an upper- or lowercase letter or underscore, which can optionally be followed by those same characters or decimal digits.

Local variable names, function parameter names and property names *must* be preceded by `$`. For example:

```hack
class Data {
  const int MAX_VALUE = 100;
  private int $value = 0;
  /* ... */
}
interface ICollection { /* ... */ }
enum Position: int {
  Top = 0;
  Bottom = 1;
  Left = 2;
  Right = 3;
  Center = 4;
}
function compute(int $val): void {
  $count = $val + 1;
  /* ... */
}
```

## Placeholder Variables
Variables beginning with `$_` are referred to as *placeholder variables*, and should be used where an assignment is structurally required but the value is unused. This is frequently used for the
[list intrinsic function](/hack/expressions-and-operators/list) and the [foreach statement](/hack/statements/foreach) - for example:

* `list($foo, $_, $bar) = baz();` if `baz()` returns a 3-element tuple but the middle element is unused
* `foreach ($container as $key => $_) { /* ... */ }` when iterating a keyed container and the values are unneeded

## The Current Instance Variable
The name `$this` is predefined inside any instance method or constructor when that method is called from within an object context.
`$this` is read-only and designates the object on which the method is being called, or the object being constructed. The type of
`$this` is [`this`](/hack/built-in-types/this).

## Reserved Names
Names beginning with two underscores (__) are reserved by the Hack language.
