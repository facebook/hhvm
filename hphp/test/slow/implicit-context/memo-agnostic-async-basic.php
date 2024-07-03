<?hh

async function printPA(): Awaitable<mixed> {
  var_dump(TestAsyncContext::getContext()); // prints 4
  return 1;
}

async function addFive(): Awaitable<void> {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  return TestAsyncContext::getContext() // returns 5
    + await TestAsyncContext::genRunWith(4, printPA<>); // returns 1
}

<<__EntryPoint>>
async function main() {
  include 'memo-agnostic-async.inc';
  var_dump(await TestAsyncContext::getContext()); // not set
  var_dump(await TestAsyncContext::genRunWith(5, addFive<>));
}
