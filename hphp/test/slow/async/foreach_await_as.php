<?hh

class DestructLogger {
  private $where;
  public function __construct($where) {
    $this->where = $where;
    echo "constructing in {$this->where}\n";
  }

  private function __destruct() {
    echo "destructing in {$this->where}\n";
  }
}

function l($obj, $where) {
  $obj->logger = new DestructLogger($where);
  return $obj;
}

async function block() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
}

async function foo($from, $to) {
  foreach (range($from, $to) as $num) {
    echo "foo $num\n";
    await block();
    yield $num;
  }
}

async function bar($from, $to) {
  echo "begin bar\n";
  foreach (l(foo($from, $to), "bar") await as $num) {
    echo "bar $num\n";
    if ($num > 47) {
      break;
    }
    yield $num => array($num * $num, $num * $num * $num);
  }
  echo "end bar\n";
}

async function baz($from, $to) {
  echo "begin baz\n";
  foreach (l(bar($from, $to), "baz") await as $key => list($value1, $value2)) {
    echo "baz $key\n";
    if ($key % 5 == 0) {
      continue;
    }
    echo "$key $value1 $value2\n";
  }
  echo "end baz\n";
}

HH\Asio\join(baz(42, 100));
