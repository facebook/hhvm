<?hh

function busy() :mixed{ sleep(0); }

class Ref {
  public function __construct(public int $cnt)[] {}
}

<<__EntryPoint>>
function main_intervaltimer() :mixed{
$x1 = new Ref(0);
$t1 = new IntervalTimer(
  0.1, 0.1,
  function() use ($x1) {
    $x1->cnt++;
  });

$x2 = new Ref(0);
$t2 = new IntervalTimer(
  0.5, 0.5,
  function() use ($x2) {
    $x2->cnt++;
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

var_dump($x1->cnt);
var_dump($x2->cnt);
var_dump($x1->cnt > $x2->cnt);
}
