The `mixed` type represents any value at all in Hack.

For example, the following function can be passed anything.

```Hack no-extract
function takes_anything(mixed $m): void {}

function call_it(): void {
  takes_anything("foo");
  takes_anything(42);
  takes_anything(new MyClass());
}
```

`mixed` is equivalent to `?nonnull`. `nonnull` represents any value
except `null`.

We recommend you avoid using `mixed` whenever you can use a more
specific type.
