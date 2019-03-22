<?hh

IncdecMagic2::$bar = new Bar;
IncdecMagic2::$foo = new Foo;

class Bar {
  protected $lol;

  public function __get($x) {

    echo "Bar heh\n";
    IncdecMagic2::$foo->asd++;
  }
}

class Foo {
  public function __get($x) {

    echo "Foo heh\n";
    IncdecMagic2::$bar->lol++; // Fatal error
  }
}

IncdecMagic2::$foo->blah++;
var_dump(IncdecMagic2::$foo);
var_dump(IncdecMagic2::$bar);

abstract final class IncdecMagic2 {
  public static $foo;
  public static $bar;
}
