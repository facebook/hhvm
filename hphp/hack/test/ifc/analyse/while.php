<?hh

<<__InferFlows>>
function simple(bool $b, string $s1, string $s2): arraykey {
  $x = "";
  while ($b) {
    $x = $s1;
    $x = $s2;
  }
  return $x;
}

<<__InferFlows>>
function union(bool $b, int $i, string $s1, string $s2): arraykey {
  $x = $i;
  while ($b) {
    $x = $s1;
    $x = $s2;
  }
  return $x;
}

<<__InferFlows>>
function breaks(bool $b, int $i, string $s1, string $s2): arraykey {
  $x = $i;
  while ($b) {
    $x = $s1;
    if ($b) break;
    $x = $s2;
  }
  return $x;
}

<<__InferFlows>>
function continues(bool $b, int $i, string $s1, string $s2): arraykey {
  $x = $i;
  while ($b) {
    $x = $s1;
    if ($b) continue;
    $x = $s2;
  }
  return $x;
}
