<?hh

async function g($i, $s): Awaitable<mixed>{
  if ($i > 1) return;
  echo "in g should be {$s} got ";
  echo TestAsyncContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "in g should be {$s} got ";
  echo TestAsyncContext::getContext()->name() . "\n";
  concurrent {
    await TestAsyncContext::genRunWith(new A, async () ==> {
      await g($i + 1, 'A');
    });
    await TestAsyncContext::genRunWith(new B, async () ==> {
      await g($i + 1, 'B');
    });
  }
  echo "done with concurrent should be {$s} got ";
  echo TestAsyncContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "done with g should be {$s} got ";
  echo TestAsyncContext::getContext()->name() . "\n";
}

async function f(): Awaitable<mixed>{
  echo "in f should be C got ";
  echo TestAsyncContext::getContext()->name() . "\n";
  concurrent {
    await g(0, 'C');
    await TestAsyncContext::genRunWith(new D, async () ==> {
      echo "in lambda should be D got ";
      echo TestAsyncContext::getContext()->name() . "\n";
      await RescheduleWaitHandle::create(
        RescheduleWaitHandle::QUEUE_DEFAULT,
        0
      );
      echo "done with lambda should be D got ";
      echo TestAsyncContext::getContext()->name() . "\n";
    });
  };
  echo "done with f should be C got ";
  echo TestAsyncContext::getContext()->name() . "\n";
}

<<__EntryPoint>>
async function main(): Awaitable<mixed>{
  include 'memo-agnostic-async.inc';

  await TestAsyncContext::genRunWith(new C, f<>);
}
