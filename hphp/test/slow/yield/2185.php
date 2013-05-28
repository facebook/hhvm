<?php

function gen(int $mode) {
  yield $mode;
  switch ($mode) {
    case 0: break;
    case 1: yield break;
    case 2: throw new Exception();
  }
  yield 47;
}

for ($mode = 0;
 $mode < 3;
 ++$mode) {
  echo "Testing mode $mode:\n";
  $gen = gen($mode);
  try {
    $gen->next();
    while ($gen->valid()) {
      var_dump($gen->current());
      $gen->next();
    }
  }
 catch (Exception $ex) {
    echo "EXCEPTION\n";
  }
  var_dump($gen->valid());
  var_dump($gen->current());
}
