<?hh

async function dead() :Awaitable<mixed>{
  echo "dead\n";
  throw new Exception('dead');
}

async function foo() :Awaitable<mixed>{
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  await dead();
}


<<__EntryPoint>>
function main_asio_callbacks_failure() :mixed{
set_error_handler(function() {
  echo "error handler\n";
  throw new Exception('error-handler');
});

AsyncFunctionWaitHandle::setOnFailCallback(function() {
  echo "on fail callback\n";
  throw new Exception('on-fail-callback');
});

try {
  HH\Asio\join(foo());
} catch (Exception $e) {
  echo "caught {$e->getMessage()}\n";
}
}
