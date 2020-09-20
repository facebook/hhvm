# Purity

### Feature Name: Pure Functions

### Start Date: August 28, 2020

### Status: Candidate

# Summary:

Hack functions that are deterministic, idempotent, and do not result in side effects.

Feature motivation:

Fixing codegen is a major initiative within Facebook. There are two currently unsolved problems:

1. Some major uses of codegen require dependency tracking in order to determine when to regenerate code. If the dependencies aren’t pure, then running codegen may have different results without reflecting changes in the file system, meaning the dependencies aren’t statically trackable.
2. A common pattern of codegen is to compute a large map and then store it to disk to avoid constantly recomputing it. Using creative automatic dependency tracking, HHVM can maintain a server-local cache of these maps that only gets recomputed when an underlying dependency changes (which is when codegen would need to run anyway). In this way, we can avoid writing these maps to disk as codegen.

# User Experience:

Pure functions will utilize the coeffects framework. As such, the way to mark a function as pure is simply this:

`<<__Pure>> function my_fn(): RetType {...}`

There are a few relatively simple restrictions on what is allowed within pure functions.

## Pure Transitivity

Pure functions may only invoke other pure functions, though may be invoked by impure functions. This all falls out naturally from the coeffects system.

```
function my_impure_fn(): void {/* terribly impure things */}
<<__Pure>> function my_pure_fn() void {
  my_impure_function(); // whoops
}
function my_other_impure_fn(): void {
  my_pure_fn(); // fine
}
```

## No Accessing Global State

Writing to global state results in side effects. Reading from global state results in the loss of idempotency. As such, accessing global state is simply wholesale banned in pure functions. This includes such things as superglobals and static members.

```
class Foo {
  public static int $foo;
  private static int $bar;

  <<__Pure>> private static function getBar(): int {
    // Static property cannot be used in a pure context. (Typing[4228])
    return self::$bar;
  }
}

<<__Pure>> function rx_func(): void {
  // Static property cannot be used in a pure context. (Typing[4228])
  Foo::$foo;
  // Superglobal $_GET cannot be used in pure function. (NastCheck[3041])
  $_GET;
}
```

## No IO

IO is effectively an example of accessing global state. The only way to do IO in hack is to utilize functions exposed by HHVM or via an intrinsics such as `echo`. The former will simply not be exposed as pure and the latter are banned operations within pure functions.

```
<<__Pure>> function pure(): void {
  // `echo` or `print` are not allowed in pure functions. Hack(4226)
  echo 'foo';
  // `echo` or `print` are not allowed in pure functions. Hack(4226)
  print 'par';
  // Pure functions can only call other pure functions. Hack(4200)
  fgets(STDIN);
}
```

## No Modifying Local Objects

By default, hack objects can have multiple references to them, including those potentially stored outside of the pure context or simply visible to another frame of execution. Due to this, modifications of objects is illegal and all objects are considered immutable within pure functions. This restriction is relaxed only for the `$this` object within constructors.

Note that this means that functions on Hack Collections modifying the collection are necessarily impure.

```
class Foo {
  private int $bar;

  // both explicit and implicit assignments here are fine
  <<__Pure>> public function __construct(int $bar, public string $baz) {
    $this->bar = $bar;
  }

  <<__Pure>> public function setBar(): void {
    // This object's property is being mutated(used as an lvalue)
    // You cannot set object properties in pure functions (Typing[4202])
    $this->bar = 42;
    // This object's property is being mutated(used as an lvalue)
    // You cannot set object properties in pure functions (Typing[4202])
    $this->bar += 17;
  }
}

<<__Pure>> function some_pure_fn(
  Foo $foo,
  Vector<string> $vector,
  Map<string, int> $map,
  Set<int> $set,
): void {
  // This object's property is being mutated(used as an lvalue)
  // You cannot set object properties in pure functions (Typing[4202])
  $foo->baz = 'monkeys';
  // Cannot append to a Hack Collection object in a pure context. (Typing[4201])
  $vector[] = 'apples';
  // Cannot assign to element of Hack Collection object via [] in a pure context. (Typing[4201])
  $vector[0] = 'bananas';
  // Pure functions can only call other pure functions. Hack(4200)
  $vector->set(0, 'bananas');
  // Cannot append to a Hack Collection object in a pure context (Typing[4201])
  $set[] = 11;
  // Cannot assign to element of Hack Collection object via [] in a pure context. (Typing[4201])
  $map['pajamas'] = 42;
}
```

Values types, however, may be "modified", as these modifications are necessarily not visible outside the current frame of execution.

```
<<__Pure>> function some_pure_fn(
  vec<string> $vec,
  dict<string, int> $dict,
  keyset<int> $keyset,
): void {
  $vec[] = 'apples';
  $vec[0] = 'bananas';
  $keyset[] = 11;
  $dict['pajamas'] = 42;
}
```

An exception made to the above rule is memoization using the `__Memoize` attribute.

## Enforced Determinism

Non-determinism is prohibited as it invalidates computational consistency. By design, these operations must utilize HHVM builtins at some point in their execution, and those builtins are not able to be pure due to their innate side-effects. Typical examples are functions such as rand and time. While it would be possible to implement a pseudo-random number generator completely without using HHVM builtins, this function would either need to be deterministic or store internal state to produce changing results. The former would result in code that is acceptable within a pure function, but not particularly useful, and the latter is illegal for reasons other than issues with determinism.

Due to the inherent non-determinism caused by Hack's asynchronous execution model, all asynchronous functions must be immediately `await`ed. `Awaitables` are only legal as the result of a function call and must be immediately unwrapped into their resultant type. Concurrent blocks are permissible.

In order to avoid issues arising from suspension of execution, it is banned for the common cases asynchronous execution and generators. HHVM will enforce that we never suspend execution of a pure function due to usage of `await`. `yield` and friends are banned syntactically within pure functions. However, an exception is made specifically for the error handler that is invoked due to notices triggered by HHVM. For the sake of execution of the overall program, we are considering the error handler to be pure in that it will not affect state visible to the context. Once the error handler is moved into HHVM, this will become even more true. It is possible that we will delay the error handler being invoked until after leaving the pure context.

# IDE experience:

The IDE experience is as above in the user experience. Users will get descriptive error messages when attempting to do illegal operations within pure functions.

# Implementation details:

See the proposal for coeffects.

This is fully enforced within HHVM. The illegal operations within a function are banned at the opcode level and will fail to emit. The transitivity requirement is enforced by calling conventions, including polymorphically-pure functions as described in the coeffects proposal.

# Design rationale and alternatives:

The major additional feature considered on top of this involves some way to add safe mutations that are tracked such that they do not result in side effects or nondeterminism. This will be discussed in more detail in a followup document.

# Drawbacks:

The lack of mutable state combined with the other restrictions of this system make it more difficult to write idiomatic hack code, and make it impossible to represent certain structures requiring generally mutable state.

# Prior art:

`constexpr` in C++.

`const fn`s in rust.

`comptime` in Zig

`noSideEffect` in Nim

Purity in more than a handful of functional languages.

# Unresolved questions:

Do we want some way to do print-debugging within pure code in sandbox mode? What about the interactive debugger? Does stepping through a pure function and modifying values cause problems?

# Future possibilities:

We can potentially add multiple different kinds of optimizations based on the knowledge that the code isn’t side-effectful as well as idempotent and deterministic.

An additional major benefit to this work is the potential to use Pure functions in constant initializer positions. This is currently a major source of unsoundness. Additionally, there are very practical applications of this to the upcoming Enum Classes feature.

## Coeffects

Once the coeffects feature is shipped, pure contexts will be integrated within it. As such the syntax will change from `<<__pure>> function(): void {...}` to `function()[pure]: void {...}` or similar, depending on the finalized version of the coeffects proposal.
