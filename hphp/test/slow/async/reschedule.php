<?hh

async function re($e) {
  echo "begin 2**$e(-1, 0, 1)\n";
  $priority = (int)pow(2, $e);
  $r = async $diff ==> {
    try {
      await RescheduleWaitHandle::create(0, $priority + $diff);
    } catch (Exception $ex) {
      echo "exception\t$priority\t$diff\t".$ex->getMessage()."\n";
    }
    echo "$e\t(2**$e)+$diff\t$priority\n";
  };
  await AwaitAllWaitHandle::fromArray(
    array($r(-1), $r(0), $r(1)),
  );
}
AwaitAllWaitHandle::fromArray(array_map(
  async $e ==> await re($e),
  [ 1, 10, 20 ],
))->join();
echo "\n\n";
AwaitAllWaitHandle::fromArray(array_map(
  async $e ==> await re($e),
  [ 1, 30, 31, 32, 33, 48, 62, 63 ],
))->join();

RescheduleWaitHandle::create(0, PHP_INT_MAX)->join();
try {
  RescheduleWaitHandle::create(0, PHP_INT_MAX + 1)->join();
} catch (Exception $ex) {
  echo "caught expected exception\n";
}

echo "fin.";
