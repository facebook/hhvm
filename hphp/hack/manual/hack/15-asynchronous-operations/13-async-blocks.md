Inside a lengthy async function, it's generally a good idea to group together data fetches that are independent of the rest of the
function. This reduces unneeded waiting for I/O.

To express this grouping inline, we would usually have to use a helper function; however, an async block allows for the immediate execution
of a grouping of code, possibly within zero-argument, async lambdas.

## Syntax

The syntax for an async block is:

```
async {
  // grouped together calls, usually await.
  < statements >
}
```

## Usage

Async blocks have two main use-cases. Remember, this is essentially syntactic sugar to make life easier.
- Inline simple async statements that would before have required a function call to execute.
- Replace the call required by an async lambda to return an actual `Awaitable<T>`.

```Hack
async function get_int_async(): Awaitable<int> {
  return 4;
}

async function get_float_async(): Awaitable<float> {
  return 1.2;
}

async function get_string_async(): Awaitable<string> {
  return "Hello";
}

async function call_async<Tv>((function(): Awaitable<Tv>) $gen): Awaitable<Tv> {
  return await $gen();
}

async function use_async_lambda(): Awaitable<void> {
  // To use an async lambda with no arguments, you would need to have a helper
  // function to return an actual Awaitable for you.
  $x = await call_async(
    async () ==> {
      $y = await get_float_async();
      $z = await get_int_async();
      return \round($y) + $z;
    },
  );
  \var_dump($x);
}

async function use_async_block(): Awaitable<void> {
  // With an async block, no helper function is needed. It is all built-in to the
  // async block itself.
  $x = await async {
    $y = await get_float_async();
    $z = await get_int_async();
    return \round($y) + $z;
  };
  \var_dump($x);
}

async function call_async_function(): Awaitable<void> {
  // Normally we have to call a simple async function and get its value, even
  // if it takes no arguments, etc.
  $x = await get_string_async();
  \var_dump($x);
}

async function use_async_block_2(): Awaitable<void> {
  // Here we can inline our function right in the async block
  $x = await async {
    return "Hello";
  };
  \var_dump($x);
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(use_async_lambda());
  \HH\Asio\join(use_async_block());
  \HH\Asio\join(call_async_function());
  \HH\Asio\join(use_async_block_2());
}
```

## Limitations

The typechecker does not allow the use of an async block immediately on the right-hand side of the `==>` in a lambda-creation expression.

In async named-functions, `async` immediately precedes `function`, which, in turn, immediately precedes the parameters. In async
lambdas, `async` also immediately precedes the parameters.

So:

```Hack no-extract
$x = async () ==> { ... } // good
$x = () ==> async { ... } // bad
```

Basically, this is just a safety guard to reduce the likelihood of unintended behavior.
