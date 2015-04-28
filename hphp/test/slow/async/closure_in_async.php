<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

async function test() {
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

HH\Asio\join(test());
