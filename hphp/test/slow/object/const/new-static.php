<?hh

class C {
  public function foo() :mixed{
    return new static();
  }
}
class D extends C {
  <<__Const>> public int $ci = 0;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    $this->ci = 1;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $d = new D();
  $d2 = $d->foo();
  echo "-- after constructor completes --\n";
  var_dump($d2);

  try {
    $d2->ci = 2;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  echo "-- at the end --\n";
  var_dump($d2);
}
