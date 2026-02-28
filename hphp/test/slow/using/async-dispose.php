<?hh

final class ScopeGuardAsyncDispose {
  final public async function __disposeAsync(): Awaitable<void> {
    await RescheduleWaitHandle::create(0, 0);
  }
}

async function one() :Awaitable<mixed>{
  await using new ScopeGuardAsyncDispose();
  throw new Exception('from one');
}

async function two() :Awaitable<mixed>{
  await using (new ScopeGuardAsyncDispose()) {
    throw new Exception('from two');
  }
}


<<__EntryPoint>>
function main_async_dispose() :mixed{
try {
  HH\Asio\join(one());
} catch (Exception $e) {
  var_dump($e->getMessage());
}

try {
  HH\Asio\join(two());
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
