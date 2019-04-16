<?hh

function busy() { return __hhvm_intrinsics\launder_value(42); }
abstract final class zeroInterval { public static $x; }

$x = 0;
$t = new IntervalTimer(
  0.0, 0.1,
  ($w) ==> {

    zeroInterval::$x++;
    echo "ping\n";
  });
$t->start();
while (zeroInterval::$x < 1) { busy(); }
$t->stop();
