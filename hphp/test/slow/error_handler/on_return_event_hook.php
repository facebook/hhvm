<?hh

class MyClass {
  public function __construct() {
    echo "constructing\n";
  }

  public function __destruct() {
    echo "destructing\n";
  }
}

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

async function test() {
  $my = new MyClass();
  await RescheduleWaitHandle::create(0, 0);
  return 42;
}

HH\Asio\join(test());
