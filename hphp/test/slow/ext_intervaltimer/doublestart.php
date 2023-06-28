<?hh
function retimer() :mixed{
  $t = new IntervalTimer(1.0, 1.0, () ==> {});
  return $t;
}


<<__EntryPoint>>
function main_doublestart() :mixed{
echo "t1\n";
$t1 = new IntervalTimer(1.0, 1.0, () ==> {});
$t1->start();
$t1->start();

$t1->stop();
$t1->stop();

echo "t2\n";
$t2 = new IntervalTimer(1.0, 1.0, () ==> {});
$t2->stop();

echo "t3\n";

$t3 = retimer();
echo "t3.2\n";
$t3->start();
echo "t3.3\n";
$t3->stop();
echo "t3.4\n";
}
