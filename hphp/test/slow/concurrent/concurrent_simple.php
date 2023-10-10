<?hh

async function foo1(): Awaitable<void> {
  concurrent {
    $x = await genx();
    $y = await geny();
    $z = await genz();
  }
  var_dump($x);
  var_dump($y);
  var_dump($z);
}

function genx(): Awaitable<int> {
  var_dump('genx');
  return async { await RescheduleWaitHandle::create(0, 0); var_dump('.'); return 42; };
}
function geny(): Awaitable<int> {
  var_dump('geny');
  return async { await RescheduleWaitHandle::create(0, 0); var_dump('.'); return 43; };
}
function genz(): Awaitable<int> {
  var_dump('genz');
  return async { await RescheduleWaitHandle::create(0, 0); var_dump('.'); return 44; };
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(foo1());
}
