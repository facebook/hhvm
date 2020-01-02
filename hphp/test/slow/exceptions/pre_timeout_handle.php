<?hh

function randomFunction() {
  rand() * rand();
}

async function genLongFunction() {
  echo "Testing handle during IO\n";
  set_pre_timeout_handler(1, ($handle) ==> {
    var_dump($handle);
  });
  await SleepWaitHandle::create(2000000);

  echo "Testing handle during IO but no args in lambda\n";
  set_pre_timeout_handler(1, () ==> {
    echo "Handle but no args in lambda\n";
  });
  await SleepWaitHandle::create(2000000);

  echo "Testing handle during CPU\n";
  set_pre_timeout_handler(1, ($handle) ==> {
    var_dump($handle);
  });
  $start = microtime(true);
  while (microtime(true) < $start + 2) {
    randomFunction();
  }
}


<<__EntryPoint>>
function main() {
  \HH\Asio\join(genLongFunction());
}
