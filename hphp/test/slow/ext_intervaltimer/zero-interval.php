<?hh

function busy() {}
$x = 0;
$t = new IntervalTimer(
  0, 0.1,
  ($w) ==> {
    global $x;
    $x++;
    echo "ping\n";
  });
$t->start();
while ($x < 1) { busy(); }
$t->stop();
