<?hh

abstract final class zeroInterval { public static $x; }
<<__EntryPoint>>
function entrypoint_zerointerval(): void {

  $x = 0;
  $t = new IntervalTimer(
    0.0, 0.1,
    ($w) ==> {

      zeroInterval::$x++;
      echo "ping\n";
    });
  $t->start();
  while (zeroInterval::$x < 1) { usleep(1); }
  $t->stop();
}
