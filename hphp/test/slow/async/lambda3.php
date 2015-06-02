<?hh

async function block() {
  await RescheduleWaitHandle::create(0, 0);
}

async function bar($id) {
  await block();
  return $id * $id;
}

async function foo() {
  await block();
  return await GenArrayWaitHandle::create(
    array_map(
      async $id ==> {
        return await bar($id);
      },
      array(1,2,3,4),
    )
  );
}

var_dump(HH\Asio\join(foo()));
