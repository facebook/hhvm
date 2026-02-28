<?hh

function log_construction<T>(T $obj): T {
  $class = get_class($obj);
  echo "constructing $class\n";
  return $obj;
}

class Base {
  public static function create($key) :mixed{
    return log_construction(new static($key));
  }

  public function __toString() :mixed{
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

async function foo() :Awaitable<mixed>{
  echo "foo\n";
  return 42;
}

async function bar() :Awaitable<mixed>{
  echo "bar before\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  echo "bar after\n";
  return 47;
}

async function gen($exit_type) :AsyncGenerator<mixed,mixed,void>{
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
    case 2: throw log_construction(new Exception("hello world"));
    case 3: {
      $x = await bar();
      echo "got bar $x\n";
      throw log_construction(new Exception("hello world"));
    }
  }
}

async function main($exit_type) :Awaitable<mixed>{
  echo "creating\n";
  $gen = log_construction(gen($exit_type));
  $step = 0;
  do {
    echo "calling next\n";
    $awaitable = log_construction(
      $step ? $gen->send(SentValue::create($step)) : $gen->next()
    );
    ++$step;
    echo "awaiting ".get_class($awaitable)."\n";
    $res = await $awaitable;
    if ($res === null) {
      echo "result: EOF\n";
    } else {
      $temp_str_40874 = (string)($res[0]);
      $temp_str_40888 = (string)($res[1]);
      echo "result: {$temp_str_40874} => {$temp_str_40888}\n";
    }
    $res = (bool)$res;
    $awaitable = null;
  } while ($res);
}


<<__EntryPoint>>
function main_async_generators() :mixed{
Exception::setTraceOptions(DEBUG_BACKTRACE_IGNORE_ARGS);
echo "start\n";
for ($exit_type = 0; $exit_type < 4; ++$exit_type) {
  echo "--------------------testing $exit_type--------------------\n";
  try {
    \HH\Asio\join(main($exit_type));
  } catch (Exception $e) {
    echo "exception: ".$e->getMessage()."\n";
    $e = null;
  }
}
echo "end\n";
}
