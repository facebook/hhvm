<?hh

async function gen1($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  return $a + 1;
}

async function gen2($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  return $a + 1;
}

async function genFoo() {
  $results = await GenArrayWaitHandle::Create(
    miarray(
      0 => gen1(1),
      1 => gen2(2),
      2 => gen1(85),
    )
  );
  $total = 0;
  foreach ($results as $val) {
    $total += $val;
  }
  return $total;
}

function main() {
  $result = genFoo()->join();
  var_dump($result);
}

main();
