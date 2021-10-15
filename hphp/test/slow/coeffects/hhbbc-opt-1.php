<?hh

class Foo {
  const ctx C = [write_props];

  function g1()[this::C] { echo "in g1\n"; }
  function g2(...$x)[this::C] { echo "in g2\n"; }
  function g3<reify T1, reify T2>(...$x)[this::C] { echo "in g3\n"; }

  static function g4()[this::C] { echo "in g4\n"; }

  function f($x)[this::C] {
    $this->g1();
    $this->g2(...$x);
    $this->g3<int, string>(...$x);
    self::g4();

    $y = $this;
    $y->g1();
  }
}

<<__EntryPoint>>
function main() {
  (new Foo())->f(vec[1,2]);
}
