# Concurrent

`concurrent` concurrently awaits all `await`s within a `concurrent` block and it works with [`await`-as-an-expression](/hack/asynchronous-operations/await-as-an-expression) as well!

Note: [concurrent doesn't mean multithreading](/hack/asynchronous-operations/introduction#limitations)

## Syntax

```hack no-extract
concurrent {
  $x = await x_async();
  await void_async();
  $sum = await y_async() + await z_async()();
}
$y = $x + $sum;
```

## Order-of-execution

Each of the statements in a `concurrent` block should be thought of as running concurrently. This means there shouldn't be any dependencies between the statements in a `concurrent` block.

Similar to `await`-as-an-expression, `concurrent` blocks don't provide a guaranteed order-of-execution between expressions being evaluated for any statement in the `concurrent` block. We guarantee that modifications to locals will happen after all other expressions resolve and will happen in the order in which they would happen outside a `concurrent` block.

For this example, we provide no guarantee on the execution order of the calls to `x()`, `y_async()` and `z_async()`. The assignments into `$result` are guaranteed to be well ordered however.

```hack no-extract
$result = vec[];
concurrent {
  $result[] = x() + await y_async();
  $result[] = await z_async();
}
```

## Exceptions

If any statement in a `concurrent` block throws, there are no guarantees about which (if any) awaited values were assigned or which exception will be propagated if more than one of them threw. For example if you have:

```hack no-extract
$x = 0;
try {
  concurrent {
    $x = await async { return 1; };
    await async { throw new Exception('foo'); };
    await async { throw new Exception('bar'); };
  }
} catch (Exception $e) {
  var_dump($x, $e->getMessage());
}
```
Then it is explicitly undefined whether `var_dump` will see `$x === 0` or `$x === 1` and whether the message will be 'foo' or 'bar'.

If you need granular exception handling, consider using nested try-catch blocks inside the concurrent block:

```hack no-extract
concurrent {
  $x = await async {
    try {
      ...
      return <success path>;
    } catch (...) {
      return <fallback>;
    }
  };
  $y = await async {
    // same here
  };
}
```
