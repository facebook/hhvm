<?hh

SetopMagic2::$bar = new Bar;
SetopMagic2::$foo = new Foo;

class Bar {
  protected $lol;

  public function __get($x) {

    echo "Bar heh\n";
    SetopMagic2::$foo->asd += 1;
  }
}

class Foo {
  public function __get($x) {

    echo "Foo heh\n";
    SetopMagic2::$bar->lol += 1; // Fatal error
  }
}

SetopMagic2::$foo->blah += 1;
var_dump(SetopMagic2::$foo);
var_dump(SetopMagic2::$bar);

abstract final class SetopMagic2 {
  public static $foo;
  public static $bar;
}
