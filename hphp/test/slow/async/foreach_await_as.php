<?hh // decl

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
  foreach (foo($from, $to) await as $num) {
    echo "bar $num\n";
    if ($num > 47) {
      break;
    }
    yield $num => varray[$num * $num, $num * $num * $num];
  }
  echo "end bar\n";
}

async function baz($from, $to) {
  echo "begin baz\n";
  foreach (bar($from, $to) await as $key => list($value1, $value2)) {
    echo "baz $key\n";
    if ($key % 5 == 0) {
      continue;
    }
    echo "$key $value1 $value2\n";
  }
  echo "end baz\n";
}


<<__EntryPoint>>
function main_foreach_await_as() {
HH\Asio\join(baz(42, 100));
}
