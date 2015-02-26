<?hh

function busy() {}

$t = new IntervalTimer(
  200, 0.6,
  () ==> {
    throw new Exception('buffalo');
  });
$t->start();
$n = microtime(true);
while (microtime(true) < $n + 1) {
  busy();
}
$t->stop();
