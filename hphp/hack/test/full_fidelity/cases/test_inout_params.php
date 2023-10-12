<?hh

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

function long_function_name_with_lots_of_args(
  int $a,
  string $b,
  inout arraykey $c,
  inout dict<int, vec<string>> $d,
  inout bool $e,
  int ...$f
): bool {
  return true;
}

function test(): void {
  $i = 42;
  $s = 'foo';
  $b = true;

  noop(inout $i);
  add1(inout $i);

  swap(inout $i, inout $s);

  $v = vec[];
  extend(inout $v, vec[0, 1, 2]);
  herp(extend<>, inout $v);

  $d = dict['derp' => dict[6 => vec['burp']]];
  $derp = () ==> 'derp';
  long_function_name_with_lots_of_args(
    42,
    'whatever',
    inout $i,
    inout $d[$derp()],
    inout $b,
    $v[0],
    ...$v
  );
}
