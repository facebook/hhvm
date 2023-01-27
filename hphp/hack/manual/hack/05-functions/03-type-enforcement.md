HHVM does a runtime type check for function arguments and return
values.

```Hack error
function takes_int(int $_): void {}

function check_parameter(): void {
  takes_int("not an int"); // runtime error.
}

function check_return_value(): int {
  return "not an int"; // runtime error.
}
```

If a type is wrong, HHVM will raise a fatal error. This is controlled
with the HHVM option `CheckReturnTypeHints`. Setting it to 0 or 1 will
disable this.
