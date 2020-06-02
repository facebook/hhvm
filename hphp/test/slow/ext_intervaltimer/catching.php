<?hh

<<__EntryPoint>>
function entrypoint_catching(): void {

  $t = new IntervalTimer(
    200.0, 0.6,
    () ==> {
      throw new Exception('buffalo');
    });
  try {
    $t->start();
    $n = microtime(true);
    while (microtime(true) < $n + 1) {
      usleep(1);
    }
    $t->stop();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
