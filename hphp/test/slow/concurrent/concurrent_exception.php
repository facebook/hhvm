<?hh

async function foo1(): Awaitable<void> {
  $x = -1;
  $y = -1;
  $z = -1;

  try {
    concurrent {
      $x = await genx();
      $y = await geny();
      await geny();
      $z = await genz();
    }
  } catch (Exception $_e) {}

  var_dump($x);
  var_dump($y);
  var_dump($z);
}

async function genx(): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  var_dump('.');
  return 42;
}
async function geny(): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  var_dump('.');
  invariant_violation("");
}
async function genz(): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  var_dump('.');
  return 43;
}

<<__EntryPoint>>
function main() :mixed{
  \HH\Asio\join(foo1());
}
