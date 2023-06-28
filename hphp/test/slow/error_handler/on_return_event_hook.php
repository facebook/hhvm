<?hh

class MyClass {
  public function __construct() {
    echo "constructing\n";
  }
}

async function test() :Awaitable<mixed>{
  $my = new MyClass();
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  return 42;
}

<<__EntryPoint>> function main(): void {
set_error_handler(
  ($errno, $errstr, $errfile, $errline, $errcontext) ==> {
    var_dump($errno);
    var_dump($errstr);
    var_dump($errfile);
    var_dump($errline);
    var_dump($errcontext);
  },
);

ResumableWaitHandle::setOnSuccessCallback(
  ($wait_handle, $result) ==> {
    var_dump($wait_handle->getName());
    var_dump($result);
    throw new Exception('hahaha');
  },
);

HH\Asio\join(test());
}
