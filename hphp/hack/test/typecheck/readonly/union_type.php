<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Baz {
  public int $prop = 1;
}
class Foo {
  public int $prop;
  public Baz $baz;
  public readonly Baz $ro_baz;
  public function __construct() {
    $this->prop = 1;
    $this->baz = new Baz();
    $this->ro_baz = new Baz();
  }
}

class Bar {
  public Baz $baz;
  public readonly Baz $ro_baz;
  public function __construct() {
    $this->baz = new Baz();
    $this->ro_baz = new Baz();
  }
}


function test(bool $b, Foo $x, Bar $y) : void {
  if ($b) {
    $z = $x;
  } else {
    $z = $y;
  }
  // $z is a union type here
  $z->baz = readonly new Baz();
  // This is ok because the prop is always readonly
  $z->ro_baz = readonly new Baz();
}

function test2<T as Foo>(T $x)  : void {
  // error, $x is Foo
  $x->baz = readonly new Baz();
}
