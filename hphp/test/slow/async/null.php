<?hh

function block() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
};

async function foo() {
  return await null;
}

async function foob() {
  await block();
  return await null;
}

async function test() {
  $a = await null;
  var_dump($a);

  $a = await foo();
  var_dump($a);

  $a = await foob();
  var_dump($a);
}

HH\Asio\join(test());

