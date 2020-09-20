<?hh

async function genString(): Awaitable<string> {
  return "hello world";
}

<<__EntryPoint>>
async function test(): Awaitable<void> {
  echo "prefix: " . (await genString()) . "\n";
}
