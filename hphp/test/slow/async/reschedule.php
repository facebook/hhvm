<?hh

async function re($e) :Awaitable<mixed>{
  echo "begin 2**$e(-1, 0, 1)\n";
  $priority = (int)pow(2, $e);
  $r = async $diff ==> {
    try {
      await RescheduleWaitHandle::create(
        RescheduleWaitHandle::QUEUE_DEFAULT,
        $priority + $diff,
      );
    } catch (Exception $ex) {
      echo "exception\t$priority\t$diff\t".$ex->getMessage()."\n";
    }
    echo "$e\t(2**$e)+$diff\t$priority\n";
  };
  await AwaitAllWaitHandle::fromVec(vec[$r(-1), $r(0), $r(1)]);
}

<<__EntryPoint>>
function main_reschedule() :mixed{
  \HH\Asio\join(AwaitAllWaitHandle::fromDict(dict(array_map(
    async $e ==> await re($e),
    vec[ 1, 10, 20 ],
  ))));
  echo "\n\n";
  \HH\Asio\join(AwaitAllWaitHandle::fromDict(dict(array_map(
    async $e ==> await re($e),
    vec[ 1, 30, 31, 32, 33, 48, 62, 63 ],
  ))));

  \HH\Asio\join(RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_DEFAULT,
    PHP_INT_MAX,
  ));
  try {
    \HH\Asio\join(RescheduleWaitHandle::create(
      RescheduleWaitHandle::QUEUE_DEFAULT,
      PHP_INT_MAX + 1,
    ));
  } catch (Exception $ex) {
    echo "caught expected exception\n";
  }

  echo "fin.";
}
