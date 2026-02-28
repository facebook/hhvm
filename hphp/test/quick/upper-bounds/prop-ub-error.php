<?hh

class Foo1 <T as num> {
  public T $x = 'a';
}

class Foo2 <T as string> {
  private T $x;

  public function __construct() {
    $this->x = 10;
  }
}

class Bar {}
class Foo3 <T as Bar> {
  <<__LateInit>> public static T $sx;
}

type Ta = num;
class Foo4 <T as Ta> {
  public T $x = 10;
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
    Foo3::$sx = 1;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    $o = new Foo4;
    $o->x = new Bar;
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
