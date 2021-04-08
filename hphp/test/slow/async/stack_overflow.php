<?hh

function stack_overflow() {
  stack_overflow();
}

async function fail_async() {
  await RescheduleWaitHandle::create(0, 0);
  stack_overflow();
}

<<__EntryPoint>>
async function main() {
  $awaitable = fail_async();
  register_postsend_function(() ==> {
    try {
      HH\Asio\join($awaitable);
    } catch (Exception $e) {
      var_dump($e->getMessage());
      var_dump($e->getFile());
      var_dump($e->getLine());
    }
  });
  await $awaitable;
}
