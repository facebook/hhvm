<?hh

function ping($w) :mixed{


  ExtIntervaltimerResetInCallback::$x++;
  ExtIntervaltimerResetInCallback::$t = new IntervalTimer(
    0.1,
    0.1,
    ($w) ==> { ping($w); }
  );
  ExtIntervaltimerResetInCallback::$t->start();
}

abstract final class ExtIntervaltimerResetInCallback {
  public static $t;
  public static $x;
}
<<__EntryPoint>>
function entrypoint_resetincallback(): void {

  ExtIntervaltimerResetInCallback::$t = null;
  ExtIntervaltimerResetInCallback::$x = 0;

  ExtIntervaltimerResetInCallback::$t = new IntervalTimer(0.1, 0.1, ($w) ==> { ping($w); });
  ExtIntervaltimerResetInCallback::$t->start();
  $n = microtime(true);
  while (ExtIntervaltimerResetInCallback::$x < 5) { usleep(1); }
  ExtIntervaltimerResetInCallback::$t->stop();
  var_dump(ExtIntervaltimerResetInCallback::$x >= 5);
}
