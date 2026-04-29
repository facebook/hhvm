# Named Parameters

:::warning
This feature is currently in [experimental mode](/hack/experimental-features/introduction/), under feature flag _"named_parameters"_.
:::

Named parameters allow you to pass arguments to a function by specifying the
parameter name at the call site, rather than relying solely on positional
ordering, and can be used to make your code more readable and maintainable.

They can be particularly helpful when passing arguments that are easy to mix up,
especially optional or "configuration-like" parameters such as booleans,
nullable values, and multiple parameters of the same type.

```hack
<<file: __EnableUnstableFeatures('named_parameters')>>

function send_message(
  string $body,
  named string $host,
  named ?int $thread_id = null,
  named bool $urgent = false,
): void {
  // ...
}

function example(): void {
  send_message("hello", host="a.fb.com");
  send_message("hello", host="b.fb.com", urgent=true);
  send_message("hello", host="c.fb.com", thread_id=123, urgent=true);
}
```

In this first example, `$body` is still positional, but `$host`, `$thread_id`
and `$urgent` must be passed as `name=value`, making the caller's intent
clearer. The latter two parameters have default values assigned to them, and are
therefore optional.

```hack
<<file: __EnableUnstableFeatures('named_parameters')>>

function schedule_retry(
  string $job,
  named int $delay_ms = 1000,
  named int $max_attempts = 3,
): void {
  // ...
}

function example(): void {
  schedule_retry("refresh-cache", delay_ms=5000, max_attempts=5);
}
```

Similarly, here in the second example, we see how named parameters can help make
the call site clearer with labels when there are multiple adjacent parameters of
the same type.

As a rule of thumb, keep obviously required inputs positional and make
easily-confused or optional tuning parameters named.

## Mixing Positional and Named Arguments

Functions can have both positional and named parameters. Named arguments are
matched by parameter name rather than by position, and explicitly passed
arguments are still evaluated from left to right in the order written at the
call site.

Hack allows positional and named arguments in the same call. For readability, it
is usually best to keep the positional arguments together at the left, and let
the named arguments follow to the right.

## Things To Keep In Mind

- Named parameters are not just documentation. Their names are part of
  the function's API.
- Renaming a named parameter is thus a breaking change for callers, and the
  same names must be preserved in method overrides and function types.
- Required named parameters must be provided, and unknown named arguments
  are errors.
- Named parameters can have default values.
- Default values for omitted named parameters should be side-effect
  free, as the runtime evaluation order might not match the declaration order.
- In ill-typed code, if multiple named arguments are wrong, the error
  you see may not correspond to the leftmost bad argument at the call
  site.
- `inout` named parameters are not supported.

## Prefer Named Parameters Over Shapes

Older code sometimes simulates named arguments by passing a shape. It is
recommended to use  named parameters when you are designing or updating an API –
they make call sites clearer, avoid unpacking boilerplate inside the callee, and
are more efficient than the shape-based pattern.

That said, shapes are still a good fit for some use cases, such as when you are
passing around configuration blobs. In these cases, the arguments can look
similar and could be passed as named parameters. However, using a shape instead
has the advantage of allowing you to pass that shape on to other functions which
named parameters wouldn't.
