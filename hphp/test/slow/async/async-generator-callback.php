<?hh

async function generator($mode) :AsyncGenerator<mixed,mixed,void>{
  switch ($mode) {
    case 0:
      return;
    case 1:
      yield 1;
      break;
    case 2:
      await reschedule();
      return;
    case 3:
      await reschedule();
      yield 3;
      break;
    case 4:
      yield 41;
      await reschedule();
      yield 42;
      break;
    case 5:
      await reschedule();
      await reschedule();
      yield 5;
      break;
    case 6:
      throw new Exception('bad 6');
    default:
      await reschedule();
      throw new Exception('bad 7');
  }
}function reschedule() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_DEFAULT,
    1,
  );
}

async function test() :Awaitable<mixed>{
  await reschedule(); // avoid polluting the first test
  for ($mode = 0; $mode < 8; $mode++) {
    printf("mode: %s\n", $mode);
    try {
      foreach (generator($mode) await as $value) {}
    } catch (Exception $e) {
    }
  }
  printf("done\n");
}


<<__EntryPoint>>
function main_async_generator_callback() :mixed{
ResumableWaitHandle::setOnCreateCallback(
  ($a, $b) ==> printf(
    "onCreate(%s, %s)...\n",
    get_class($a),
    get_class($b),
  ),
);
ResumableWaitHandle::setOnAwaitCallback(
  ($a, $b) ==> printf(
    "onAwait(%s, %s)...\n",
    get_class($a),
    get_class($b),
  ),
);
ResumableWaitHandle::setOnSuccessCallback(
  ($a) ==> printf(
    "onSuccess(%s)...\n",
    get_class($a),
  ),
);
ResumableWaitHandle::setOnFailCallback(
  ($a) ==> printf(
    "onFail(%s)...\n",
    get_class($a),
  ),
);
;
HH\Asio\join(test());
}
