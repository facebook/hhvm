<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

async function gen1($a) {
  $v1 = "v1-gen1";
  await RescheduleWaitHandle::Create(1, 1);
  return $a + 1;
}

async function gen2($a) {
  $v1 = "v1-gen2";
  await RescheduleWaitHandle::Create(1, 1);
  $x = gen1($a)->join();
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
  genFoo($a)->join();
}

main(42);
