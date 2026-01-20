# Awaitables

An *awaitable* is the key construct in `async` code. An awaitable is a first-class object that represents a possibly asynchronous
operation that may or may not have completed. We `await` the awaitable until the operation has completed.

An `Awaitable` represents a particular execution; this means that awaiting the
same awaitable twice **will not** execute the code twice. For example,
while the result of both `await`s below is `42`, the `print()` call (and the
`return`) only happen once:

```hack
$x = async { print("Hello, world\n"); return 42; };
\var_dump(await $x);
\var_dump(await $x);
```

This can be surprising when the result depends on the call stack; [exceptions](/hack/asynchronous-operations/exceptions)
are the most common case of this.

## `Awaitable`

Awaitables are represented by the interface called `Awaitable`. While there are several classes that implement `Awaitable`, there is no
need to concern ourselves with their implementation details. `Awaitable` is the only interface we need.

The type returned from an async function is `Awaitable<T>`, where `T` is the final result type (e.g., `int`) of the awaited value.

```hack
async function foo(): Awaitable<int> {
  throw new Exception('unimplemented');
}

async function demo(): Awaitable<void> {
  $x = foo();         // $x will be an Awaitable<int>
  $x = await foo();   // $x will be an int
}
```

```hack
async function f(): Awaitable<int> {
  return 2;
}

// We call f() and get back an Awaitable<int>
// Once the function is finished executing and we await the awaitable (or in
// this case, explicitly join since this call is not in an async function) to get
// the explicit result of the function call, we will get back 2.

<<__EntryPoint>>
function join_main(): void {
  var_dump(\HH\Asio\join(f()));
}
```

All `async` functions must return an `Awaitable<T>`. Calling an `async` function will therefore yield an object implementing the `Awaitable`
interface, and we must `await` or `join` it to obtain an end result from the operation. When we `await`, we are pausing the current task until
the operation associated with the `Awaitable` handle is complete, leaving other tasks free to continue executing. `join` is similar; however it
blocks all other operations from completing until the `Awaitable` has returned.

## Awaiting

In most cases, we will prefer to `await` an `Awaitable`, so that other tasks can execute while our blocking operation completes.  Note however,
that only `async` functions can yield control to other asyncs, so `await` may therefore only be used in an `async` function.  For other locations,
such as a `main` block, we will need to use `join`, as will be shown below.

### Batching Awaitables

Many times, we will `await` on one `Awaitable`, get the result, and move on. For example:

```hack
async function foo(): Awaitable<int> {
  return 3;
}

<<__EntryPoint>>
async function single_awaitable_main(): Awaitable<void> {
  $aw = foo(); // awaitable of type Awaitable<int>
  $result = await $aw; // an int after $aw completes
  var_dump($result);
}
```

We will normally see something like `await f();` which combines the retrieval of the awaitable with the waiting and retrieving of the result
of that awaitable. The example above separates it out for illustration purposes.

At other times, we will gather a bunch of awaitables and `await` them all before moving on.

Here we are using one of the library helper-functions in order to batch a bunch of awaitables together to then `await` upon:
* `HH\Lib\Vec\from_async`: vec of awaitables with consecutive integer keys
* `HH\Lib\Dict\from_async`: dict of awaitables with integer or string keys

```hack
async function quads(float $n): Awaitable<float> {
  return $n * 4.0;
}

<<__EntryPoint>>
async function quads_m(): Awaitable<void> {
  $awaitables = dict['five' => quads(5.0), 'nine' => quads(9.0)];
  $results = await Dict\from_async($awaitables);

  \var_dump($results['five']); // float(20)
  \var_dump($results['nine']); // float(36)
}
```

## Join

Sometimes we want to get a result out of an awaitable when the function we are in is *not* `async`. For this there is `HH\Asio\join`, which
takes an `Awaitable` and blocks until it resolves to a result.

This means that invocations of async functions from the top-level scope cannot be awaited, and must be joined.

```hack
async function get_raw(string $url): Awaitable<string> {
  return await \HH\Asio\curl_exec($url);
}

<<__EntryPoint>>
function join_main(): void {
  $result = \HH\Asio\join(get_raw("http://www.example.com"));
  \var_dump(\substr($result, 0, 10));
}
```

We should **not** call `join` inside an `async` function. This would defeat the purpose of `async`, as the awaitable and any dependencies will
run to completion synchronously, stopping any other awaitables from running.
