A supertype has one or more subtypes, and while any operation permitted on a value of some supertype is also permitted on a value of any of
its subtypes, the reverse is not true. For example, the type `num` is a supertype of `int` and `float`, and while addition and subtraction are
well defined for all three types, bit shifting requires integer operands. As such, a `num` cannot be bit-shifted directly. (Similar situations
occur with `arraykey` and its subtypes `int` and `string`, with nullable types and their subtypes, and with `mixed` and its subtypes.)

Certain program elements are capable of changing the type of an expression using what is called *type refinement*. Consider the following:

```Hack
function f1(?int $p): void {
  //  $x = $p % 3;       // rejected; % not defined for ?int
  if ($p is int) { // type refinement occurs; $p has type int
    $x = $p % 3; // accepted; % defined for int
  }
}
```

When the function starts execution, `$p` contains `null` or some `int`. However, the type of the expression `$p` is not known to be `int`, so
it is not safe to allow the `%` operator to be applied. When the test `is int` is applied to `$p`, a type refinement occurs in
which the type of the expression `$p` is changed to `int` **for the true path of the `if` statement only**. As such, the `%` operator can
be applied. However, once execution flows out of the `if` statement, the type of the expression `$p` is `?int`.

Consider the following:

```Hack
function f2(?int $p): void {
  if ($p is null) { // type refinement occurs; $p has type null
    //    $x = $p % 3;      // rejected; % not defined for null
  } else { // type refinement occurs; $p has type int
    $x = $p % 3; // accepted; % defined for int
  }
}
```

The first assignment is rejected, not because we don't know `$p`'s type, but because we know its type is not `int`. See how an opposite
type refinement occurs with the `else`.  Similarly, we can write the following:

```Hack
function f3(?int $p): void {
  if (!$p is null) { // type refinement occurs; $p has type int
    $x = $p % 3; // accepted; % defined for int
  }

  if ($p is nonnull) { // type refinement occurs; $p has type int
    $x = $p % 3; // accepted; % defined for int
  }
}
```

Consider the following example that contains multiple selection criteria:

```Hack
function f4(?num $p): void {
  if (($p is int) || ($p is float)) {
    //    $x = $p**2;    // rejected
  }
}
```

**An implementation is not required to produce the correct type refinement when using multiple criteria directly.**

The following constructs involve type refinement:
* When used as the controlling expression in an `if`, `while`, or `for` statement, the operators `==`, `!=`, `===`, and `!==` when used
with one operand of `null`, `is`, and simple assignment `=`. [Note that if `$x` is an expression of some nullable type, the
logical test `if ($x)` is equivalent to `if ($x is nonnull)`.]
* The operators `&&`, `||`, and `?:`.
* The intrinsic function `invariant`.
* Some built-in functions like `Shapes::keyExists()` and `\HH\is_any_array()` have special typechecking rules, but others, like `is_string()` and `is_null()` don't.

Thus far, all the examples use the value of an expression that designates a parameter (which is a local variable). Consider the following
case, which involves a property:

```Hack
class C {
  private ?int $p = 8; // holds an int, but type is ?int
  public function m(): void {
    if ($this->p is int) { // type refinement occurs; $this->p is int
      $x = $this->p << 2; // allowed; type is int
      $this->n(); // could involve a permanent type refinement on $p
      //      $x = $this->p << 2;   // disallowed; might no longer be int
    }
  }
  public function n(): void { /* ... */ }
}
```

Inside the true path of the `if` statement, even though we know that `$this->p` is an `int` to begin with, once any method in this class
is called, the implementation must assume that method could have caused a type refinement on anything currently in scope. As a result,
the second attempt to left shift is rejected.
