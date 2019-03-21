<?hh

function block() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function num() {
  var_dump(func_num_args());
  await block();
  var_dump(func_num_args());
}

<<__EntryPoint>>
function main_func_get_arg() {
;

HH\Asio\join(num("a", "b", "c"));
}
