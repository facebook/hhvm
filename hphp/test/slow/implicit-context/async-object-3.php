<?hh

async function g($i, $s)[zoned] :Awaitable<mixed>{
  if ($i > 1) return;
  echo "in g should be {$s} got ";
  echo ClassContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "in g should be {$s} got ";
  echo ClassContext::getContext()->name() . "\n";
  concurrent {
    await ClassContext::genStart(new A, async () ==> {
      await g($i + 1, 'A');
    });
    await ClassContext::genStart(new B, async () ==> {
      await g($i + 1, 'B');
    });
  }
  echo "done with concurrent should be {$s} got ";
  echo ClassContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "done with g should be {$s} got ";
  echo ClassContext::getContext()->name() . "\n";
}

async function f()[zoned] :Awaitable<mixed>{
  echo "in f should be C got ";
  echo ClassContext::getContext()->name() . "\n";
  concurrent {
    await g(0, 'C');
    await ClassContext::genStart(new D, async () ==> {
      echo "in lambda should be D got ";
      echo ClassContext::getContext()->name() . "\n";
      await RescheduleWaitHandle::create(
        RescheduleWaitHandle::QUEUE_DEFAULT,
        0
      );
      echo "done with lambda should be D got ";
      echo ClassContext::getContext()->name() . "\n";
    });
  };
  echo "done with f should be C got ";
  echo ClassContext::getContext()->name() . "\n";
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  await ClassContext::genStart(new C, f<>);
}
