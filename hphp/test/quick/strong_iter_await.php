<?hh

async function foo() {
  $x = range(0, 10);
  foreach ($x as &$xs) {
    $xs *= 2;
    await SleepWaitHandle::create(100000);
  }
  echo "foo\n";
}

async function bar() {
  echo "bar\n";
}

async function main() {
  $wait_handles = Vector {};
  $wait_handles[] = foo()->getWaitHandle();
  $wait_handles[] = bar()->getWaitHandle();
  return GenVectorWaitHandle::create($wait_handles);
}

async function main2() {
  $x = main();
  $x->join();
}

main2()->join();
