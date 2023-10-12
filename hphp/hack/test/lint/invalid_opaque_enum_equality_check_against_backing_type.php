<?hh // strict

enum Foo : int {
  FOO = 1;
}

function test1(Foo $f, int $i): void {
  // This is invalid, but may evaluate to true at runtime. The user should
  // satisfy the typechecker by using `Foo::assert` or `Foo::coerce` before
  // comparing.
  if ($f === $i) {
    var_dump($f);
  }
}

function test2(vec<Foo> $f, vec<int> $i): void {
  // This is invalid, but may evaluate to true at runtime.
  if ($f === $i) {
    var_dump($f);
  }
}

function test3(vec<int> $is, Foo $f): void {
  // This is invalid, but may evaluate to true at runtime.
  if (HH\Lib\C\contains($is, $f)) {
    var_dump($f);
  }
}

function test4(keyset<int> $is, Foo $f): void {
  // This is invalid, but may evaluate to true at runtime.
  if (HH\Lib\C\contains_key($is, $f)) {
    var_dump($f);
  }
}
