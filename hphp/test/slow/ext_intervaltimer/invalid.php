<?hh

$t = new IntervalTimer(0.5, 0.5, 'clowntown');
$t->start();
$n = microtime(true);
while (microtime(true) < $n + 0.8) {
}
$t->stop();
