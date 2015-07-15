<?hh

async function blockme(int $n): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  return $n;
}

function f() {
try {
  $handle = blockme(1);
  var_dump(HH\Asio\join($handle));

  $handle = blockme(2);
  HH\Asio\join(AwaitAllWaitHandle::fromArray(array($handle)));
  var_dump($handle->result());
  var_dump(HH\Asio\result($handle));

  $handle = blockme(3);
  var_dump($handle->result());
} catch (InvalidOperationException $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
}
f();
