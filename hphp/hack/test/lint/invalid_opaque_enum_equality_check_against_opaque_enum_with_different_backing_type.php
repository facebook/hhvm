<?hh

enum Foo : int {
  FOO = 1;
}

enum Bar : string {
  BAR = 'BAR';
}

function test1(Foo $f, Bar $b): void {
  // This is invalid and will always evaluate to false at runtime.
  if ($f === $b) {
    var_dump($f);
  }
}

function test2(vec<Foo> $f, vec<Bar> $b): void {
  // This is invalid and will always evaluate to false at runtime (unless the
  // vecs are both empty).
  if ($f === $b) {
    var_dump($f);
  }
}

function test3(vec<Bar> $bs, Foo $f): void {
  // This is invalid and will always evaluate to false at runtime.
  if (HH\Lib\C\contains($bs, $f)) {
    var_dump($f);
  }
}

function test4(keyset<Bar> $bs, Foo $f): void {
  // This is invalid and will always evaluate to false at runtime.
  if (HH\Lib\C\contains_key($bs, $f)) {
    var_dump($f);
  }
}
