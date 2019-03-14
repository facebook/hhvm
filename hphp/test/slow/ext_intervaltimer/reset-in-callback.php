<?hh

function busy() { return __hhvm_intrinsics\launder_value(42); }

ExtIntervaltimerResetInCallback::$t = null;
ExtIntervaltimerResetInCallback::$x = 0;
function ping($w) {


  ExtIntervaltimerResetInCallback::$x++;
  ExtIntervaltimerResetInCallback::$t = new IntervalTimer(
    0.1,
    0.1,
    ($w) ==> { ping($w); }
  );
  ExtIntervaltimerResetInCallback::$t->start();
};

ExtIntervaltimerResetInCallback::$t = new IntervalTimer(0.1, 0.1, ($w) ==> { ping($w); });
ExtIntervaltimerResetInCallback::$t->start();
$n = microtime(true);
while (ExtIntervaltimerResetInCallback::$x < 5) { busy(); }
ExtIntervaltimerResetInCallback::$t->stop();
var_dump(ExtIntervaltimerResetInCallback::$x >= 5);

abstract final class ExtIntervaltimerResetInCallback {
  public static $t;
  public static $x;
}
