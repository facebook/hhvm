<?hh

async function foo1(): Awaitable<void> {
  concurrent {
    $x = (await genx()) + (await geny());
    $y = (await genz());
  }
  var_dump($x + $y);
}

function genx(): Awaitable<int> {
  var_dump('x');
  return async { await RescheduleWaitHandle::create(0, 0); var_dump('.'); return 42; };
}
function geny(): Awaitable<int> {
  var_dump('x');
  return async { await RescheduleWaitHandle::create(0, 0); var_dump('.'); return 43; };
}
function genz(): Awaitable<int> {
  var_dump('x');
  return async { await RescheduleWaitHandle::create(0, 0); var_dump('.'); return 44; };
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(foo1());
}
