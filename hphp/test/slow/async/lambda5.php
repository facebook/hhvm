<?hh

async function block() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function foo($a) :Awaitable<mixed>{
  $fn = async ($b) ==> {
    $c = 0;
    while (true) {
      await block();
      yield $a * $b + $c;
      ++$c;
    }
  };

  $gen = $fn(47, 26);
  do {
    $next = await $gen->next();
    $next = $next[1];
    echo "$next\n";
  } while ($next < 2000);
}


<<__EntryPoint>>
function main_lambda5() :mixed{
HH\Asio\join(foo(42));
}
