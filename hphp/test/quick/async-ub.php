<?hh
type Bar = int;
async function foobar1<T as Bar>(<<__Soft>>?T $b): <<__Soft>>Awaitable<?T> {
  return $b + 1;
}

async function await_float() :Awaitable<mixed>{
  return 0.0;
}

function foobar2<T as Bar>(): Awaitable<T> {
  return await_float();
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  var_dump(await foobar1(1.2));
  var_dump(await foobar2()); // No runtime error, checking Awaitable
}
