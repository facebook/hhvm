<?hh


<<__EntryPoint>>
function main_invalid() {
$t = new IntervalTimer(0.5, 0.5, 'clowntown');
$t->start();
$n = microtime(true);
while (microtime(true) < $n + 0.8) {
  sleep(0);
}
$t->stop();
}
