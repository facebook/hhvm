<?hh

async function f($n = 'D')[zoned] :Awaitable<mixed>{
  echo "Expecting {$n} got " . ClassContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "Expecting {$n} got " . ClassContext::getContext()->name() . "\n";
}

async function g()[zoned] :Awaitable<mixed>{
  await ClassContext::genStart(new C, async () ==> {
    echo 'Expecting C got ' . ClassContext::getContext()->name() . "\n";
    // Async entry point already has an instance of the scheduler
    // lets add another
    \HH\Asio\join(ClassContext::genStart(new D, f<>));
    echo 'Expecting C got ' . ClassContext::getContext()->name() . "\n";
  });
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  await ClassContext::genStart(new A, async () ==> {
    concurrent {
      await f('A');
      await g();
    }
  });
}
