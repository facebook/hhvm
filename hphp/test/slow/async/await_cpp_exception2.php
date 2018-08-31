<?hh // decl

class ExitOnDestruct {
  private function __destruct() {
    echo "exiting...\n";
    exit(1);
  }
}

class DoubleDecRef {
  private function __destruct() {
    echo "decref\n";
  }
}

async function block() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

function foo() {
  static $wh = null;
  return $wh = $wh ?: block();
}

async function evil($local) {
  $dep = foo();
  new ExitOnDestruct();
  await $dep;
}


<<__EntryPoint>>
function main_await_cpp_exception2() {
register_postsend_function(function() {
  echo "postsend start\n";
  HH\Asio\join(foo());
  echo "postsend end\n";
});

HH\Asio\join(evil(new DoubleDecRef()));
}
