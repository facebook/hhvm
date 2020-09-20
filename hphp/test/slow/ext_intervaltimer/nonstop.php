<?hh

function x() {
  $t = new IntervalTimer(1.0, 1.0, () ==> {});
  $t->start();
}

<<__EntryPoint>>
function main_nonstop() {
x();
echo "OK\n";

$t = new IntervalTimer(1.0, 1.0, () ==> {});
$t->start();
echo "OK\n";
}
