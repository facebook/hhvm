<?hh


async function gen1($a) {
  $v1 = "v1-gen1";
  await RescheduleWaitHandle::Create(0, 0);
  return $a + 1;
}

async function gen2($a) {
  $v1 = "v1-gen2";
  await RescheduleWaitHandle::Create(0, $a);
  $x = HH\Asio\join(gen1($a));
  return $x;
}

async function genBar($a) {
  $v1 = "v1-genBar";
  var_dump($a);
  return $a + 2;
}

async function genFoo($a) {
  $v1 = "v1-genFoo";
  $a++;
  await GenArrayWaitHandle::Create(
    array(
      genBar($a),
      gen2($a + 2),
      gen2($a + 3)
    )
  );
}

function main($a) {
  HH\Asio\join(genFoo($a));
}

main(42);
