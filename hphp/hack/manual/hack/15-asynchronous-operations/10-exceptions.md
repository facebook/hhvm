# Exceptions

In general, an async operation has the following pattern:
* Call an `async` function
* Get an awaitable back
* `await` the awaitable to get a result

If an exception is thrown within an `async` function body, the function does not
technically throw - it returns an `Awaitable` that throws when resolved. This
means that if an `Awaitable` is resolved multiple times, the same exception
object instance will be thrown every time; as it is the same object every time,
its data will be unchanged, **including backtraces**.

```hack
async function exception_thrower(): Awaitable<void> {
  throw new \Exception("Return exception handle");
}

async function basic_exception(): Awaitable<void> {
  // the handle does not throw, but result will be an Exception objection.
  // Remember, this is the same as:
  //   $handle = exception_thrower();
  //   await $handle;
  await exception_thrower();
}

<<__EntryPoint>>
function main(): void {
  HH\Asio\join(basic_exception());
}
```

The use of `from_async` ignores any successful awaitable results and just throw an exception of one of the
awaitable results, if one of the results was an exception.

```hack
async function exception_thrower(): Awaitable<void> {
  throw new \Exception("Return exception handle");
}

async function non_exception_thrower(): Awaitable<int> {
  return 2;
}

async function multiple_waithandle_exception(): Awaitable<void> {
  $handles = vec[exception_thrower(), non_exception_thrower()];
  // You will get a fatal error here with the exception thrown
  $results = await Vec\from_async($handles);
  // This won't happen
  var_dump($results);
}

<<__EntryPoint>>
function main(): void {
  HH\Asio\join(multiple_waithandle_exception());
}
```

To get around this, and get the successful results as well, we can use the [utility function](/hack/asynchronous-operations/utility-functions)
[`HH\Asio\wrap`](/apis/Functions/HH.Asio/wrap/). It takes an awaitable and returns the expected result or the exception
if one was thrown. The exception it gives back is of the type [`ResultOrExceptionWrapper`](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/).

```hack no-extract
namespace HH\Asio {
  interface ResultOrExceptionWrapper<T> {
    public function isSucceeded(): bool;
    public function isFailed(): bool;
    public function getResult(): T;
    public function getException(): Exception;
  }
}
```

Taking the example above and using the wrapping mechanism, this is what the code looks like:

```hack
async function exception_thrower(): Awaitable<void> {
  throw new Exception();
}

async function non_exception_thrower(): Awaitable<int> {
  return 2;
}

async function wrapping_exceptions(): Awaitable<void> {
  $handles = vec[
    HH\Asio\wrap(exception_thrower()),
    HH\Asio\wrap(non_exception_thrower()),
  ];
  // Since we wrapped, the results will contain both the exception and the
  // integer result
  $results = await Vec\from_async($handles);
  var_dump($results);
}

<<__EntryPoint>>
function main(): void {
  HH\Asio\join(wrapping_exceptions());
}
```

## Memoized Async Exceptions
Because [`__Memoize`](/hack/attributes/predefined-attributes#__memoize) caches the awaitable itself, **if an async function
is memoized and throws, you will get the same exception backtrace on every
failed call**.

For example, both times an exception is caught here, `foo` is in the backtrace,
but `bar` is not, as the call to `foo` led to the creation of the memoized
awaitable:

```hack
<<__Memoize>>
async function throw_something(): Awaitable<int> {
  throw new Exception();
}

async function foo(): Awaitable<void> {
  await throw_something();
}

async function bar(): Awaitable<void> {
  await throw_something();
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  try {
    await foo();
  } catch (Exception $e) {
    var_dump($e->getTrace()[2] as shape('function' => string, ...)['function']);
  }
  try {
    await bar();
  } catch (Exception $e) {
    var_dump($e->getTrace()[2] as shape('function' => string, ...)['function']);
  }
}
```
