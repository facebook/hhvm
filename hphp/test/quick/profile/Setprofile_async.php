<?hh

// Test to confirm that we have the proper number of enter/exit
// profiling calls for async functions, and that they are balanced. No
// real logic in here to do that, just simple printing. If you cause
// the output to change, inspect the results carefully to ensure we
// have a balanced enter/exit for every call.

async function gen1($a) {
  error_log('In gen1');
  error_log('Finished in gen1');
  await RescheduleWaitHandle::Create(0, 0); // simulate blocking I/O
  return $a + 1;
}

async function gen2($a) {
  error_log('In gen2');
  await RescheduleWaitHandle::Create(0, $a); // simulate blocking I/O
  $x = HH\Asio\join(gen1($a));
  error_log('Finished in gen2');
  return $x;
}

async function genBar($a) {
  error_log('In genBar');
  var_dump($a);
  await RescheduleWaitHandle::Create(0, $a); // simulate blocking I/O
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

function proffunc($event, $name, $info) {
  echo "** $event $name\n";
}

function main($a) {
  fb_setprofile('proffunc');
  $result = HH\Asio\join(genFoo($a));
  var_dump($result);
  fb_setprofile(null);
}

main(42);
