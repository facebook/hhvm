<?hh

async function genEager(...$args) {
  echo "eager execution\n";
  return $args;
}

function block() { // simulates blocking I/O
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
};

function generator(...$args) {
  yield 'a';
  yield $args;
  yield break;
};

async function genBlocking(...$args) {
  echo "before blocking\n";
  var_dump($args);
  await block();
  echo "after blocking\n";
  return $args;
}

function main() {
  echo "* eager async *\n";
  $result = HH\Asio\join(genEager('a', 'b', 'c'));
  var_dump($result);

  echo "* blocking async *\n";
  $result = HH\Asio\join(genBlocking('a', 'b', 'c'));
  var_dump($result);

  echo "* generator *\n";
  foreach (generator('a', 'b', 'c') as $yielded) {
    var_dump($yielded);
  }
}
main();
