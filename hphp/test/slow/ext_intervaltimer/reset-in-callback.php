<?hh

function busy() {}

$t = null;
$x = 0;
function ping($w) {
  global $t;
  global $x;
  $x++;
  $t = new IntervalTimer(
    0.1,
    0.1,
    ($w) ==> { ping($w); }
  );
  $t->start();
};

$t = new IntervalTimer(0.1, 0.1, ($w) ==> { ping($w); });
$t->start();
$n = microtime(true);
while ($x < 5) { busy(); }
$t->stop();
var_dump($x >= 5);
