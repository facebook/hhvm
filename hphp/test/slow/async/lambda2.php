<?hh

async function block() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function bar($id) {
  await block();
  return $id * $id;
}

async function foo() {
  await block();
  return await \HH\Asio\m(
    array_map(
      async $id ==> await bar($id),
      varray[1,2,3,4],
    )
  );
}


<<__EntryPoint>>
function main_lambda2() {
var_dump(HH\Asio\join(foo()));
}
