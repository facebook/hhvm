<?hh

async function genEager($a) {
  try {
    echo "eager execution\n";
    return 10;
  } finally {
    echo "finally\n";
  }
}

function block() { // simulates blocking I/O
  return RescheduleWaitHandle::create(1,1);
};

async function genBlocking($a) {
  try {
    echo "before blocking\n";
    await block();
    echo "after blocking\n";
    return 10;
  } finally {
    echo "finally\n";
  }
}

function main() {
  echo "* eager async *\n";
  $result = genEager(42)->join();
  var_dump($result);

  echo "* blocking async *\n";
  $result = genBlocking(42)->join();
  var_dump($result);
}
main();
