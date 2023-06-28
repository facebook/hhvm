<?hh

interface Fooable {}
interface Barable{}

class Foobar implements Fooable, Barable {}
class Nobar implements Fooable {}

class Foo <T1 as int, T2 as Fooable as Barable> {
  public ?T1 $x = 3.14;
  public ?T2 $y = 1;
  public static T2 $sy = 10;
  private T2 $z;
  <<__Soft, __LateInit>> public T2 $w;

  public function __construct() {
    $this->z = 1;
  }
  public function setW(mixed $p) :mixed{
    $this->w = $p;
  }
}

class Bar <reify T as int> {
  public T $x = 1;
}

<<__EntryPoint>> function main() :mixed{
  $o = new Foo;
  $o->y = 'a';
  $o->y = new Nobar;
  $o->y = new Foobar;
  $o->y = null;
  Foo::$sy = null;
  Foo::$sy = new Nobar;
  Foo::$sy = new Foobar;
  $o->setW(new Foobar);
  $o->setW(new Nobar);
  $p = new Bar<string>;
  $p->x = 'a'; // upper-bound warning
}
