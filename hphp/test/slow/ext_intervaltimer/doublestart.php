<?hh

echo "t1\n";
$t1 = new IntervalTimer(1, 1, () ==> {});
$t1->start();
$t1->start();

$t1->stop();
$t1->stop();

echo "t2\n";
$t2 = new IntervalTimer(1, 1, () ==> {});
$t2->stop();

echo "t3\n";
function retimer() {
  $t = new IntervalTimer(1, 1, () ==> {});
  return $t;
}

$t3 = retimer();
echo "t3.2\n";
$t3->start();
echo "t3.3\n";
$t3->stop();
echo "t3.4\n";
