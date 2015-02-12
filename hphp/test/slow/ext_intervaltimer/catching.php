<?hh

$t = new IntervalTimer(
  0.6, 0.6,
  () ==> {
    throw new Exception('buffalo');
  });
try {
  $t->start();
  $n = microtime(true);
  while (microtime(true) < $n + 1) {
  }
  $t->stop();
} catch (Exception $e) {
  echo $e->getMessage()."\n";
}
