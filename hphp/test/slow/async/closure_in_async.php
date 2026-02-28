<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function test() :Awaitable<mixed>{
  $closure = function ($a) {
    echo "closure\n";
    return $a + 1;
  };

  $asyncclosure = async function ($a) {
    echo "asyncclosure\n";
    return $a + 2;
  };

  var_dump($closure(0));
  var_dump(HH\Asio\join($asyncclosure(1)));
  await block();
  var_dump($closure(2));
  var_dump(HH\Asio\join($asyncclosure(3)));
}

<<__EntryPoint>>
function main_closure_in_async() :mixed{
;

HH\Asio\join(test());
}
