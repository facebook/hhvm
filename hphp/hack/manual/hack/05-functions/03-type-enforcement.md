# Type Enforcement

HHVM does a runtime type check for function arguments, return
values, and properties.

```hack error
function takes_int(int $_): void {}

function check_parameter(): void {
  takes_int("not an int"); // runtime error.
}

function check_return_value(): int {
  return "not an int"; // runtime error.
}
```

If a type is wrong, HHVM will raise an error.

## Generics

### Erased Generics

For erased generics, Hack enforces the upper bound (for example
`T as num`) in the typechecker. The generic constraint is erased at
runtime.

```hack
function add_one<T as num>(T $x): num {
  return $x + 1;
}

function erased_generic_runtime_example(dynamic $d): void {
  add_one<float>($d);
}

function call_erased_generic_runtime_example(): void {
  erased_generic_runtime_example(41); // no runtime error.
  erased_generic_runtime_example("not a num"); // runtime error.
}
```

The same is true for bounded class generics: the upper bound is
checked by the typechecker, but is not enforced at runtime.

```hack
class NumBox<T as num> {
  public function __construct(public T $value) {}

  public function set(T $value): void {
    $this->value = $value;
  }
}

function bounded_class_generic_example(dynamic $d): void {
  $box = new NumBox<float>($d);
  $box->set($d);
}

function call_bounded_class_generic_example(): void {
  bounded_class_generic_example(41); // no runtime error.
  bounded_class_generic_example("not a num"); // runtime error.
}
```

### Reified generics

For reified generics, HHVM can enforce the type argument that was
instantiated for `T` at runtime.

```hack
function takes_num<reify T as num>(T $x): T {
  return $x;
}

function reified_generic_runtime_example(dynamic $d): void {
  takes_num<float>($d);
}

function call_reified_generic_runtime_example(): void {
  reified_generic_runtime_example(41); // runtime error.
  reified_generic_runtime_example("not a num"); // runtime error.
}
```

A direct mismatch against a reified type argument also throws at
runtime.

```hack error
function reified_direct<reify T as num>(T $x): T {
  return $x;
}

function call_reified_direct(): void {
  reified_direct<int>(3.14); // runtime error.
  reified_direct<int>("not a num"); // runtime error.
}
```

## Async Functions

In general, generic type arguments are erased and not enforced at
runtime. Functions declared with the `async` keyword are a special
case: HHVM enforces the inner type in the declared `Awaitable<T>`
return type as part of return type checking.

```hack error
async function returns_int(): Awaitable<int> {
  return "not an int"; // runtime error.
}
```
