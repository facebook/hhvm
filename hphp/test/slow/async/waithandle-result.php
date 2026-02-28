<?hh

async function blockme(int $n): Awaitable<int> {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  return $n;
}

function f() :mixed{
try {
  $handle = blockme(1);
  var_dump(HH\Asio\join($handle));

  $handle = blockme(2);
  HH\Asio\join(AwaitAllWaitHandle::fromVec(vec[$handle]));
  var_dump(HH\Asio\result($handle));

  $handle = blockme(3);
  var_dump(HH\Asio\result($handle));
} catch (InvalidOperationException $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
}

<<__EntryPoint>>
function main_waithandle_result() :mixed{
f();
}
