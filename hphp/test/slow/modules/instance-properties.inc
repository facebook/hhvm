<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;

class Foo {
  internal int $x = 5;

  public function __construct(internal vec<int> $v) {
    $this->v = $v;
  }
}

function ok(): void {
  $x = new Foo(vec[2]);
  $x->x = 4;
  var_dump($x->x);
}
