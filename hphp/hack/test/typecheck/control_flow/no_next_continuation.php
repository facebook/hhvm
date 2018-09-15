<?hh // strict

function test1(bool $b, int $x): bool {
  if (true) {
    $x = $b;
  } else {
    return $x;
  }
  return $x;
}

function test2(bool $b): void {
  if (false && $b) {
  }
}

function test3(int $x): int {
  if (false) {
    return $x;
  }
  return $x;
}

function test4(int $a, bool $b): void {
  true ? $a : $b;
}
