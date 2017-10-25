<?hh // strict

function noop<T>(inout T $_): void {}

function add1(inout int $x): void {
  $x += 1;
}

function swap(inout $a, inout $b) {
  $tmp = $a;
  $a = $b;
  $b = $tmp;
}

function extend<T>(inout vec<T> $dst, vec<T> $src): bool {
  if (!$src) {
    return false;
  }
  foreach ($src as $e) {
    $dst[] = $e;
  }
  return true;
}
