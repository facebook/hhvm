<?hh

<<__InferFlows>>
function binop(int $x, int $y): int {
  return $x + 1;
}

<<__InferFlows>>
function assign0(int $arg): int {
  $x = $arg;
  $x = 10;
  return $x;
}

<<__InferFlows>>
function assign1(int $arg): int {
  $x = $arg;
  $x = $x + 10;
  return $x;
}

<<__InferFlows>>
function condition(int $a0, int $a1, int $a2): int {
  if ($a2 > 0) {
    $x = $a0;
  } else {
    $x = $a1;
  }
  return $x;
}
