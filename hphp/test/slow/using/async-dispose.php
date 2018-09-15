<?hh

final class ScopeGuardAsyncDispose {
  final public async function __disposeAsync(): Awaitable<void> {
    await RescheduleWaitHandle::Create(0, 0);
  }
}

async function one() {
  await using new ScopeGuardAsyncDispose();
  throw new Exception('from one');
}

async function two() {
  await using (new ScopeGuardAsyncDispose()) {
    throw new Exception('from two');
  }
}


<<__EntryPoint>>
function main_async_dispose() {
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
