To strike a balance between flexibility, latency, and performance, we require that the `await`s only appear in **unconditionally consumed expression positions**. This means that from the closest statement, the result of the `await` must be used under all non-throwing cases. This is important because all `await`s for a statement will run together, we don't want to over-run code if its result might not be utilized.

All `await`s within the same statement will execute concurrently.

## Examples

```Hack no-extract
$sum =
  await x_async() +      // yes!
  await y_async();       // yes!
$tuple = tuple(
  await foo_async(),     // yes!
  42,
);
$result = foo(
  await bar_async(),     // yes!
  await baz_async(),     // yes!
);
if (await x_async()) {   // yes!
  // Conditional but separate statement
  await y_async();       // yes!
}
$x =
  await x_async() &&     // yes!
  // Conditional expression
  await y_async();       // no!
$y = await x_async()     // yes!
  ? await y_async()      // no!
  : await z_async();     // no!
$x = await async {       // yes!
  await x_async();       // yes!
}
```

## Order-of-execution

Similar to other aspects of `await`, we do not guarantee an order of execution of the expressions within a statement that contains `await`, but you should assume it could be significantly different than if the `await` wasn't present. **If you want stronger guarantees over order-of-execution, separate `await`s into their own statements.**

In this example, you should make no assumptions about the order in which `a_async()`, `b()`, `c_async()` or `d()` are executed, but you can assume that both `await`'ed functions (`a_async()` and `c_async()`) will be concurrently awaited.

```Hack no-extract
$x = foo(
  await a_async(),
  b(),
  await c_async(),
  d(),
);
```

## Limitations

To further help protect against depending on order-of-execution, we've
banned assignment or updating variables as-an-expression for
statements that contain an `await`.

```Hack no-extract
// Yes!
$x = await x_async();
// No, assignment as an expression
await foo_async($x = 42);
// No, we even disallow separate assignment
(await bar_async()) + ($x = 42);
// Yes!
$x = f(inout $y, await x_async());
// Yes, embedded call w/ inout is considered an expression
await bar_async(baz(inout $x));
```

Hack doesn't currently support nested `await`s.

```Hack error
// Syntax error.
$y = await foo_async(await bar_async());

// Must be written as this instead.
$x = await bar_async();
$y = await foo_async($x);
```
