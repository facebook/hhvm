<?hh

function busy() { return __hhvm_intrinsics\launder_value(42); }

$x = 0;
$t = new IntervalTimer(
  0, 0.1,
  ($w) ==> {
    global $x;
    $x++;
    echo "ping\n";
  });
$t->start();
while ($x < 1) { busy(); }
$t->stop();
