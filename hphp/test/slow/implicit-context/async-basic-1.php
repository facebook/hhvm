<?hh

async function addFive() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  var_dump(await IntContext::genStart(5, addFive<>));
}
