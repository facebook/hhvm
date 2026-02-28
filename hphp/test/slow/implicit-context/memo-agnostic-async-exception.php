<?hh

async function throwExn(): Awaitable<mixed>{
  RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
  throw new Exception();
}

<<__EntryPoint>>
async function main(): Awaitable<mixed>{
  include 'memo-agnostic-async.inc';

  await TestAsyncContext::genRunWith(1, () ==> async {
    try {
      await TestAsyncContext::genRunWith(2, () ==> async {
        RescheduleWaitHandle::create(
          RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
          1,
        );
        var_dump(TestAsyncContext::getContext());
        await throwExn();
      });
    } catch (Exception $e) {
      var_dump('caught!');
      var_dump(TestAsyncContext::getContext());
    }
  });
}
