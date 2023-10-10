<?hh

async function foo(
): Awaitable<string> {
  return await async {
    return "Hello World";
  };
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  $x = await foo();
  echo $x;
}
