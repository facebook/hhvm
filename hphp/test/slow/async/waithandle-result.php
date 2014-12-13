<?hh

async function blockme(int $n): Awaitable<int> {
  await RescheduleWaitHandle::create(0, 0);
  return $n;
}

try {
  $handle = blockme(1);
  var_dump($handle->join());

  $handle = blockme(2);
  AwaitAllWaitHandle::fromArray(array($handle))->join();
  var_dump($handle->result());

  $handle = blockme(3);
  var_dump($handle->result());
} catch (InvalidOperationException $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
