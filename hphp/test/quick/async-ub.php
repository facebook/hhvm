<?hh
type Bar = int;
async function foobar1<T as Bar>(?T $b): Awaitable<?T> {
  return $b + 1;
}

async function await_float() {
  return 0.0;
}

function foobar2<T as Bar>(): Awaitable<T> {
  return await_float();
}

<<__EntryPoint>>
async function main() {
  var_dump(await foobar1(1.2));
  var_dump(await foobar2()); // No runtime error, checking Awaitable
}
