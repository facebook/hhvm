<?hh

enum Foo : int {
  FOO = 1;
}

function test1(Foo $f, string $s): void {
  // This is invalid and will always evaluate to false at runtime.
  if ($f === $s) {
    var_dump($f);
  }
}

function test2(vec<Foo> $f, vec<string> $s): void {
  // This is invalid and will always evaluate to false at runtime (unless the
  // vecs are both empty).
  if ($f === $s) {
    var_dump($f);
  }
}

function test3(vec<string> $strs, Foo $f): void {
  // This is invalid and will always evaluate to false at runtime.
  if (HH\Lib\C\contains($strs, $f)) {
    var_dump($f);
  }
}

function test4(keyset<string> $strs, Foo $f): void {
  // This is invalid and will always evaluate to false at runtime.
  if (HH\Lib\C\contains_key($strs, $f)) {
    var_dump($f);
  }
}
