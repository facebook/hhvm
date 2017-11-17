<?hh // strict

function noop<T>(inout T $_): void {}

function add1(inout int $x): void {
  $x += 1;
}

function swap<T>(inout T $a, inout T $b): void {
  $tmp = $a;
  $a = $b;
  $b = $tmp;
}

function extend<T>(inout vec<T> $dst, Traversable<T> $src): bool {
  if (!$src) {
    return false;
  }
  foreach ($src as $e) {
    $dst[] = $e;
  }
  return true;
}

function herp(
  (function(inout vec<int>, Container<int>): mixed) $f,
  inout vec<int> $dst,
): void {
  $f(inout $dst, keyset[5]);
}

function test(): void {
  $i = 42;
  $s = 'foo';

  noop(inout $i);
  add1(inout $i);

  swap(inout $i, inout $s);
  swap(&$i, inout $s);
  swap(inout $i, &$s);
  swap(&$i, &$s);

  $v = vec[];
  extend(inout $v, vec[0, 1, 2]);
  extend(&$v, dict['spam' => 3, 'eggs' => 4]);
  herp(fun('extend'), inout $v);
}
