<?hh

function x() {
  $t = new IntervalTimer(1, 1, () ==> {});
  $t->start();
}
x();
echo "OK\n";

$t = new IntervalTimer(1, 1, () ==> {});
$t->start();
echo "OK\n";
