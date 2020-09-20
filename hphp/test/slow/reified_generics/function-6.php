<?hh

class C<reify T> {}

async function f(): Awaitable<C<(function():int)>> {
  return new C<(function():int)>();
}

async function g(): Awaitable<C<(function():string)>> {
  return new C<(function():int)>();
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  await f();
  echo "ok\n";
  await g();
  echo "ok\n";
}
