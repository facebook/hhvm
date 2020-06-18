<?hh

include 'async-implicit.inc';

async function addFive() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
async function main() {
  var_dump(await IntContext::genStart(5, fun('addFive')));
}
