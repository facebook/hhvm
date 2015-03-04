<?hh

function busy() {}

$t = new IntervalTimer(0, 0.1, ($w) ==> { echo "ping\n"; });
$t->start();
$n = microtime(true);
while (microtime(true) < $n + 0.3) { busy(); }
$t->stop();
