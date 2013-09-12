<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

async function test() {
  $arr3 = array(1,2,3);
  $arr5 = array(1,2,3,4,5);
  foreach ($arr3 as $x) {
    foreach ($arr5 as $y) {
      if ($x>=2 && $y==3) {
        await block();
      };
      echo $y;
    }
  }
}

async function test2() {
  await block();
  $arr3 = array(1,2,3);
  $arr5 = array(1,2,3,4,5);
  foreach ($arr3 as $x) {
    foreach ($arr5 as $y) {
      echo $y;
    }
  }
}

$t = test();
echo "\n";
$t->join();
echo "\n";
$t2 = test2();
echo "\n";
$t2->join();
echo "\n";
