<?hh

async function g() {
  echo "in g 1 should be C got ";
  echo ClassContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "in g 2 should be C got ";
  echo ClassContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "done with g should be C got ";
  echo ClassContext::getContext()->name() . "\n";
}

async function h() {
  echo "in h should be C got ";
  echo ClassContext::getContext()->name() . "\n";
  await ClassContext::genStart(new D, async () ==> {
    echo "in lambda should be D got ";
    echo ClassContext::getContext()->name() . "\n";
    await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
    echo "done with lambda should be D got ";
    echo ClassContext::getContext()->name() . "\n";
  });
  echo "done with h should be C got ";
  echo ClassContext::getContext()->name() . "\n";
}

async function f() {
  echo "in f should be C got ";
  echo ClassContext::getContext()->name() . "\n";
  concurrent {
    await g();
    await h();
  };
  echo "done with f should be C got ";
  echo ClassContext::getContext()->name() . "\n";
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  await ClassContext::genStart(new C, fun('f'));
}
