<?hh

async function block() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function awaitva(...$args) {
  await AwaitAllWaitHandle::fromVec(vec($args));
  return array_map($x ==> HH\Asio\join($x), $args);
}

async function foo() {
  list(
    $res1,
    $res2,
    $res3,
  ) = await awaitva(
    block(),
    async {
      await block();
      return 42;
    },
    async {
      await block();
      return 47;
    },
  );

  return HH\Lib\Legacy_FIXME\cast_for_arithmetic($res1) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($res2) * HH\Lib\Legacy_FIXME\cast_for_arithmetic($res3);
}


<<__EntryPoint>>
function main_immed_lambda2() {
var_dump(HH\Asio\join(foo()));
}
