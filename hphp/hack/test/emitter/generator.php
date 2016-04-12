<?hh // strict

function range_(int $lo, int $hi): Generator<int, int, void> {
  for ($i = $lo; $i < $hi; $i++) {
    yield $i;
    if ($i == 5) {
      yield break;
    }
  }
}

function keygen(): Generator<string, int, void> {
  yield 'a' => 1;
  yield 'b' => 2;
  yield 'c' => 3;
}

function broken_keygen(): Generator<string, int, void> {
  yield 'a' => 1;
  /* HH_FIXME[4110] */
  yield '2';
}

function zero(): int { return 0; }
function yield_val(): Generator<int, int, int> {
  $i = 2;
  $welp = array();
  while ($i != 16) {
    $welp[zero()] = yield $i*$i;
    $x = $welp[0];
    $i = $x === null ? $i+1 : $x;
  }
}

function test(): void {
  foreach (range_(1, 10) as $i) {
    echo $i, "\n";
  }
  foreach (keygen() as $k => $v) {
    echo $k, " => ", $v, "\n";
  }
  foreach (broken_keygen() as $k => $v) {
    echo $k, " => ", $v, "\n";
  }
  foreach (yield_val() as $i) {
    echo $i, "\n";
  }

  $k = yield_val();
  while ($k->valid()) {
    $i = $k->current();
    echo $i, "\n";
    $k->send($i);
  }

}
