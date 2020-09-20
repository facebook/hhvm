<?hh // strict

class Foo {
  public function f(int $x): int {
    return 1;
  }
  public static function g(int $x): int {
    return 2;
  }
  public function h(): int {
    return 3;
  }
}

function incr(int $x): int {
  return $x + 1;
}

function test(): void {
  var_dump(array_map(fun('incr'), varray[1, 2, 3]));

  $x = new Foo();
  var_dump(array_map(inst_meth($x, 'f'), varray[1, 2, 3]));
  var_dump(array_map(meth_caller('Foo', 'h'), varray[$x, $x, $x]));

  var_dump(array_map(class_meth('Foo', 'g'), varray[1, 2, 3]));
}
