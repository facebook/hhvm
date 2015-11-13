<?hh

class ExitOnDestruct {
  private function __destruct() {
    echo "exiting\n";
    exit(1);
  }
}

async function block() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function crash() {
  await block();
  $block = block();
  $x = new ExitOnDestruct();
  echo "triggering destructor\n";
  $x = null;
  echo "will exit once suspend hook is called\n";
  await $block;
  echo "should have exited!\n";
}

HH\Asio\join(crash());
