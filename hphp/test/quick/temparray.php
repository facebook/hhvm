<?

function dynString() {
  static $state = 0x71177beef7337;
  // Defeat any conceivable constant-folding smarts
  $state = ($state << 3) ^ 022707;
  return (string)$state;
}

function dynArray($n) {
  foreach (range(0, $n) as $i) {
    $a[] = dynString();
  }
  return $a;
}

var_dump(dynArray(2)[1]);

