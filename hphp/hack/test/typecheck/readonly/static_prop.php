<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Baz {
  public int $prop = 1;
}
class Foo {
  public int $prop;
  public Baz $baz;
  public static ?Baz $baz_static = null;
  public static readonly ?Baz $baz_ro_static = null;
  public function __construct() {
    $this->prop = 1;
    $this->baz = new Baz();
  }
  public function test(): void {
    self::$baz_static = readonly new Baz();
  }

}

function test() : void {
  Foo::$baz_static = readonly new Baz();
  $a = Foo::$baz_ro_static;
  // ok
  Foo::$baz_ro_static = readonly new Baz();
  // ok
  Foo::$baz_ro_static = new Baz();
  // ok
  Foo::$baz_static = new Baz();
  if ($a is nonnull) {
    $a->prop = 4;
  }
}
