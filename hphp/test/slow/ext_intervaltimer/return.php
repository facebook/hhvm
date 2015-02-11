<?hh
$t1 = new IntervalTimer(1, 1, () ==> {echo "t1\n";});
$t1->start();

$t1->stop();
$t1->stop();

function retimer() {
  $t3 = new IntervalTimer(1, 1, () ==> {echo "t3\n";});
  return $t3;
}

$t3 = retimer();
$t3->start();
$t3->stop();
echo "OK\n";
