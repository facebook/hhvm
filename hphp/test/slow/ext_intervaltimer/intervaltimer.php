<?hh

function busy() {}

$x1 = null;
$t1 = new IntervalTimer(
  0.1, 0.1,
  function() use (&$x1) {
    $x1++;
  });

$x2 = null;
$t2 = new IntervalTimer(
  0.5, 0.5,
  function() use (&$x2) {
    $x2++;
  });

$t1->start();
$t2->start();
$now = microtime(true);
while (microtime(true) < $now + 1) {
  // busy wait
  busy();
}
$t2->stop();
$t1->stop();

var_dump($x1);
var_dump($x2);
var_dump($x1 > $x2);
