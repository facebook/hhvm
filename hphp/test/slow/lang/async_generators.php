<?hh

class Logger {
  public static function attach($obj) {
    $obj->logger = new Logger(get_class($obj));
    return $obj;
  }

  private $what;
  private function __construct($what) {
    $this->what = $what;
    echo "constructing {$this->what}\n";
  }

  private function __destruct() {
    echo "destructing {$this->what}\n";
  }
}

class Base {
  public static function create($key) {
    return Logger::attach(new static($key));
  }

  public function __toString() {
    return (string)$this->key;
  }

  private $key;
  private function __construct($key) {
    $this->key = $key;
  }
}

class Key extends Base {}
class Value extends Base {}
class SentValue extends Base {}

async function foo() {
  echo "foo\n";
  return 42;
}

async function bar() {
  echo "bar before\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  echo "bar after\n";
  return 47;
}

async function gen($exit_type) {
  echo "step 1\n";
  $val = yield Value::create(1);
  echo "step 2: got $val\n";
  $val = null;
  $val = yield Key::create(2) => Value::create(3);
  echo "step 3: got $val\n";
  $val = null;
  $x = await foo();
  echo "got foo $x\n";
  $val = yield Value::create($x);
  echo "step 4: got $val\n";
  $val = null;
  $x = await bar();
  echo "got bar $x\n";
  $val = yield Value::create($x);
  echo "step 5: got $val\n";
  $val = null;
  $x = await bar();
  echo "got bar $x\n";
  $y = await bar();
  echo "got bar $y\n";
  $val = yield Key::create($x) => Value::create($y);
  echo "step 6: got $val\n";
  switch ($exit_type) {
    case 0: return;
    case 1: {
      $x = await bar();
      echo "got bar $x\n";
      return;
    }
    case 2: throw Logger::attach(new Exception("hello world"));
    case 3: {
      $x = await bar();
      echo "got bar $x\n";
      throw Logger::attach(new Exception("hello world"));
    }
  }
}

async function main($exit_type) {
  echo "creating\n";
  $gen = Logger::attach(gen($exit_type));
  $step = 0;
  do {
    echo "calling next\n";
    $awaitable = Logger::attach(
      $step ? $gen->send(SentValue::create($step)) : $gen->next()
    );
    ++$step;
    echo "awaiting ".get_class($awaitable)."\n";
    $res = await $awaitable;
    if ($res === null) {
      echo "result: EOF\n";
    } else {
      echo "result: {$res[0]} => {$res[1]}\n";
    }
    $res = (bool)$res;
    $awaitable = null;
  } while ($res);
}

function my_join(WaitHandle $awaitable) {
  return $awaitable->join();
}

echo "start\n";
for ($exit_type = 0; $exit_type < 4; ++$exit_type) {
  echo "--------------------testing $exit_type--------------------\n";
  try {
    main($exit_type)->join();
  } catch (Exception $e) {
    echo "exception: ".$e->getMessage()."\n";
    $e = null;
  }
}
echo "end\n";
