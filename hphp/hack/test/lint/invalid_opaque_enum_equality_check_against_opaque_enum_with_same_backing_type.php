<?hh

enum Foo : int {
  FOO = 1;
}

enum Bar : int {
  BAR = 1;
  BAZ = 2;
}

function test1(Foo $f, Bar $b): void {
  // This is invalid, but may evaluate to true at runtime.
  if ($f === $b) {
    var_dump($f);
  }
}

function test2(vec<Bar> $bs, Foo $f): void {
  // This is invalid, but may evaluate to true at runtime.
  if (HH\Lib\C\contains($bs, $f)) {
    var_dump($f);
  }
}

function test3(keyset<Bar> $bs, Foo $f): void {
  // This is invalid, but may evaluate to true at runtime.
  if (HH\Lib\C\contains_key($bs, $f)) {
    var_dump($f);
  }
}

function test4(vec<Foo> $f, vec<Bar> $b): void {
  // This is invalid, but may evaluate to true at runtime.
  if ($f === $b) {
    var_dump($f);
  }
}
