<?hh

class Foo {
  static mixed $bar;
}

async function y($n = 'D') {
  // the first time this is called from z() with the context of D...
  // then x() finishes eagerly, resetting the context to C...
  // but then the existing Awaitable for y() gets used
  // for AwaitAllWaitHandle, so this function will be the last thing to
  // run under the scheduler, leaving it with the context of D
  echo "Expecting {$n} got " . ClassContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "Expecting {$n} got " . ClassContext::getContext()->name() . "\n";
}

async function z() {
  Foo::$bar = y('D'); // no await
}

async function x() {
  ClassContext::genStart(new D, z<>);
}

async function g() {
  await ClassContext::genStart(new C, async () ==> {
    echo 'Expecting C got ' . ClassContext::getContext()->name() . "\n";
    // Async entry point already has an instance of the scheduler
    // lets add another
    $a = ClassContext::genStart(new D, y<>);
    echo 'Expecting C got ' . ClassContext::getContext()->name() . "\n";
    \HH\Asio\join(AwaitAllWaitHandle::fromVec(vec[x(), Foo::$bar]));
    echo 'Expecting C got ' . ClassContext::getContext()->name() . "\n";
  });
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  await ClassContext::genStart(new A, async () ==> {
    concurrent {
      await y('A');
      await g();
    }
  });
}
