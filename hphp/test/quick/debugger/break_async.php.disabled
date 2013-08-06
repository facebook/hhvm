<?hh

// Test showing breakpoints on async functions in the debugger.

async function gen1($a) {
  error_log('In gen1');
  error_log('Finished in gen1');
  return $a + 1;
}

async function gen2($a) {
  error_log('In gen2');
  $x = gen1($a)->join();
  error_log('Finished in gen2');
  return $x;
}

async function genBar($a) {
  error_log('In genBar');
  var_dump($a);
  error_log('Finished in genBar');
  return $a + 2;
}

async function genFoo($a) {
  error_log('In genFoo');
  var_dump($a);
  $a++;
  list($x, $y) = await GenArrayWaitHandle::Create(
    array(
      genBar($a),
      genBar($a + 1),
      gen2($a + 2)
    )
  );
  var_dump($x);
  var_dump($y);
  error_log('Finished in genFoo');
  return $x + $y;
}

function main($a) {
  $result = genFoo($a)->join();
  var_dump($result);
}

main(42);

