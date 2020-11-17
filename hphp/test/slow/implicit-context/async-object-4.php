<?hh

async function g() {
  echo "in g should be D got ";
  echo ClassContext::getContext()->name() . "\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "resumed g should be D got ";
  echo ClassContext::getContext()->name() . "\n";
}

async function f() {
  echo "in f should be C got ";
  echo ClassContext::getContext()->name() . "\n";
  $a = ClassContext::genStart(new D, g<>);
  echo "before await in f should be C got ";
  echo ClassContext::getContext()->name() . "\n";
  await $a;
  echo "done with f should be C got ";
  echo ClassContext::getContext()->name() . "\n";
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  await ClassContext::genStart(new C, f<>);
}
