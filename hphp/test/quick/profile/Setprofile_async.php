<?hh

// Test to confirm that we have the proper number of enter/exit
// profiling calls for async functions, and that they are balanced. No
// real logic in here to do that, just simple printing. If you cause
// the output to change, inspect the results carefully to ensure we
// have a balanced enter/exit for every call.

async function genList(...$args) :Awaitable<mixed>{
  await AwaitAllWaitHandle::fromVec(vec($args));
  return array_map($wh ==> \HH\Asio\result($wh), $args);
}

async function gen1($a) :Awaitable<mixed>{
  error_log('In gen1');
  error_log('Finished in gen1');
  await RescheduleWaitHandle::create(0, 0); // simulate blocking I/O
  return $a + 1;
}

async function gen2($a) :Awaitable<mixed>{
  error_log('In gen2');
  await RescheduleWaitHandle::create(0, $a); // simulate blocking I/O
  $x = HH\Asio\join(gen1($a));
  error_log('Finished in gen2');
  return $x;
}

async function genBar($a) :Awaitable<mixed>{
  error_log('In genBar');
  var_dump($a);
  await RescheduleWaitHandle::create(0, $a); // simulate blocking I/O
  error_log('Finished in genBar');
  return $a + 2;
}

async function genFoo($a) :Awaitable<mixed>{
  error_log('In genFoo');
  var_dump($a);
  $a++;
  list($x, $y) = await genList(
    genBar($a),
    genBar($a + 1),
    gen2($a + 2)
  );
  var_dump($x);
  var_dump($y);
  error_log('Finished in genFoo');
  return $x + $y;
}

function proffunc($event, $name, $info) :mixed{
  echo "** $event $name\n";
}

function main($a) :mixed{
  fb_setprofile(
    proffunc<>,
    SETPROFILE_FLAGS_RESUME_AWARE | SETPROFILE_FLAGS_DEFAULT,
  );
  $result = HH\Asio\join(genFoo($a));
  var_dump($result);
  fb_setprofile(null);
}
<<__EntryPoint>> function main_entry(): void {
main(42);
}
