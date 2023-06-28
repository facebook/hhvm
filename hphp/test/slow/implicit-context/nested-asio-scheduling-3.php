<?hh

class Foo {
  public static mixed $bar;
}

async function y() :Awaitable<mixed>{
  // the first time this is called from z() with the context of D...
  // then x() finishes eagerly, resetting the context to C...
  // but then the existing Awaitable for y() gets used
  // for AwaitAllWaitHandle, so this function will be the last thing to
  // run under the scheduler, leaving it with the context of D
  echo "Expecting D got " . ClassContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "Expecting D got " . ClassContext::getContext()->name() . "\n";
}

async function z() :Awaitable<mixed>{
  Foo::$bar = y('D'); // no await
}

async function x() :Awaitable<mixed>{
  ClassContext::genStart(new D, z<>);
}

async function f()[zoned, globals] :Awaitable<mixed>{
  \HH\Asio\join(AwaitAllWaitHandle::fromVec(vec[x(), Foo::$bar]));
  echo 'Expecting C got ' . ClassContext::getContext()->name() . "\n";
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';
  await ClassContext::genStart(new C, f<>);
}
