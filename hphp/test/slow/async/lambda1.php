<?hh

async function block() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function foo() {
  await block();
  return await GenArrayWaitHandle::create(
    array_map(
      async $id ==> $id * $id,
      array(1,2,3,4),
    )
  );
}

var_dump(HH\Asio\join(foo()));
