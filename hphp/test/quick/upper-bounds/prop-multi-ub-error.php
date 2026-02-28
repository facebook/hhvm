<?hh

interface Fooable {}
interface Barable {}

class FooBar implements Fooable, Barable {}
class NoFoo implements Barable {}

class Foo1 <T as num as int> {
  public T $x = 'a';
}

class Foo2 <T as Barable as Fooable> {
  private T $x;

  public function __construct() {
    $this->x = 10;
  }
}

class Foo3 <T as Fooable as Barable> {
  <<__LateInit>> public static T $sx;
}

type Ta = Barable;
class Foo4 <T as Ta as Fooable> {
  public ?T $x = null;
}

<<__EntryPoint>> function main() :mixed{
  set_error_handler(
    (int $errno, string $errstr) ==> {
      if ($errno === E_RECOVERABLE_ERROR) {
        throw new Exception($errstr);
      } else {
        echo "Warning/notice not expected.\n";
      }
    }
  );

  try {
    $o = new Foo1;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $o = new Foo2;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    Foo3::$sx = new FooBar;
    Foo3::$sx = 1;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $o = new Foo4;
    $o->x = new NoFoo;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
