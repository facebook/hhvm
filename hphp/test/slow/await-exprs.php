<?hh

<<file:__EnableUnstableFeatures('allow_extended_await_syntax')>>

async function f(int $x): Awaitable<int> {
  echo "start f($x)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end f($x)\n";
  return $x + 1;
}

async function g(int $x): Awaitable<int> {
  echo "start g($x)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end g($x)\n";
  return $x;
}

<<__EntryPoint>>
async function main() {
  var_dump(await f(1) + await g(2));
  var_dump(await f(await g(1)) + await f(await g(10)));
  var_dump(await f(await f(await g(1)) + await f(await g(2))));
  var_dump(await f(1) + await f(await g(2)));
}
