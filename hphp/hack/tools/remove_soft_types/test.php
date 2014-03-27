<?hh


class C {
  public static function f(
    @array<int, string> $foo,
    $whatever
  ): void {}
}

function g(@string $bar): void {}

function h(): @Vector<int> {
}

class :foo:bar:hello-world {
  public function i(@int $x) {
  }
}

C::f(null);
g(null);
$f = <foo:bar:hello-world>toto</foo:bar:hello-world>;
$f->i("titi");
