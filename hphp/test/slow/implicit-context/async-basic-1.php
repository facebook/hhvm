<?hh

async function addFive() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  var_dump(await IntContext::genStart(5, addFive<>));
}
