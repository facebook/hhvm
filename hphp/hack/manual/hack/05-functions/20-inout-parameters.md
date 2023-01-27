Hack functions normally pass arguments by value. `inout` provides
"copy-in, copy-out" arguments, which allow you to modify the variable
in the caller.

```Hack
function takes_inout(inout int $x): void {
  $x = 1;
}

function call_it(): void {
  $num = 0;
  takes_inout(inout $num);

  // $num is now 1.
}
```

This is similar to copy-by-reference, but the copy-out only happens
when the function returns. If the function throws an exception, no
changes occur.

```Hack
function takes_inout(inout int $x): void {
  $x = 1;
  throw new Exception();
}

<<__EntryPoint>>
function call_it(): void {
  $num = 0;
  try {
    takes_inout(inout $num);
  } catch (Exception $_) {
  }

  // $num is still 0.
}
```

`inout` must be written in both the function signature and the
function call. This is enforced in the typechecker and at runtime.

## Indexing with `inout`

In addition to local variables, `inout` supports indexes in value
types.

```Hack
function set_to_value(inout int $item, int $value): void {
  $item = $value;
}

function use_it(): void {
  $items = vec[10, 11, 12];
  $index = 1;
  set_to_value(inout $items[$index], 42);

  // $items is now vec[10, 42, 12].
}
```

This works for any value type: `vec`, `dict`, `keyset` or `array`. You
can also do nested access e.g. `inout $foo[$y][z()]['stuff']`.

## Dynamic Usage

`inout` is a different calling convention, so dynamic calls will not
work with `inout` parameters. For example, you cannot use
`meth_caller`, `call_user_func` or `ReflectionFunction::invoke`.

`unset` on a `inout` parameter will set the value to `null`. This is
not recommended, and will raise a warning when the function returns.
