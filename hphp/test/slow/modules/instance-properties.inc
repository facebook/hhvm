<?hh

module foo;

class Foo {
  internal int $x = 5;
  static internal int $y = 0;
  static internal vec<int> $sv = vec[5];
  public function __construct(internal vec<int> $v) {
    $this->v = $v;
  }
}

function ok(): void {
  $x = new Foo(vec[2]);
  $x->x = 4;
  Foo::$sv[] = 5;
  $y = $x::$sv;
  $y[] = 7;
  var_dump($x->x);
}
