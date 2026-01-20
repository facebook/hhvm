# Noreturn

A function that never returns a value can be annotated with the
`noreturn` type. A `noreturn` function either loops forever, throws an
an error, or calls another `noreturn` function.

```hack
function something_went_wrong(): noreturn {
  throw new Exception('something went wrong');
}
```

`invariant_violation` is an example of a library function with a `noreturn` type.

`noreturn` informs the typesystem that code execution can not continue past a certain line.
In combination with a conditional, you can refine variables, since the typesystem will take note.
This is actually how [invariant](/hack/expressions-and-operators/invariant) is [implemented](/apis/Functions/HH/invariant).

```hack
<<__EntryPoint>>
async function main_async(): Awaitable<void> {
  $nullable_int = '_' ? 0 : null;
  if (!($nullable_int is nonnull)) {
    invariant_violation('$nullable_int must not be null');
  }
  // If we didn't fall into the if above, $nullable_int must be an int.
  takes_int($nullable_int);
}

function takes_int(int $int): void {
  echo $int;
}
```

If you want to, you can also use [nothing](/hack/built-in-types/nothing) instead. This allows you use the return value of the function.
This makes it more explicit to the reader of your code that you are depending on the fact that this function influences typechecking.

```hack
function i_am_a_noreturn_function(): noreturn {
  throw new Exception('stop right here');
}

function i_return_nothing(): nothing {
  i_am_a_noreturn_function();
}

const ?int NULLABLE_INT = 0;

async function main_async(): Awaitable<void> {
  example_noreturn();
  example_nothing();
}

function example_noreturn(): int {
  $nullable_int = NULLABLE_INT;
  if ($nullable_int is null) {
    i_am_a_noreturn_function();
  }
  return $nullable_int;
}

function example_nothing(): int {
  $nullable_int = NULLABLE_INT;
  if ($nullable_int is null) {
    return i_return_nothing();
  }
  return $nullable_int;
}
```

In this example the `noreturn` function is named very plain, so you can understand that this refines.
However in the `nothing` version you don't need to know the signature of `i_return_nothing()`
to understand that `$nullable_int` will not be null after the if, since you can see the `return`.
