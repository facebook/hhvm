<?hh

function retimer() :mixed{
  $t3 = new IntervalTimer(1.0, 1.0, () ==> {echo "t3\n";});
  return $t3;
}

<<__EntryPoint>>
function main_return() :mixed{
$t1 = new IntervalTimer(1.0, 1.0, () ==> {echo "t1\n";});
$t1->start();

$t1->stop();
$t1->stop();

$t3 = retimer();
$t3->start();
$t3->stop();
echo "OK\n";
}
