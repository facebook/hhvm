<?hh

<<__Const>>
class C {
  public int $ci = 0;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    $this->ci = 1;
  }
}

class D extends C {}

class E {
  public int $ci = 0;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    $this->ci = 1;
  }
}

function dynamic_construct($class) :mixed{
  echo "---- class $class ----\n";

  $x = new $class();
  echo "-- after constructor completes --\n";
  var_dump($x);

  try {
    $x->ci = 2;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  echo "-- at the end --\n";
  var_dump($x);
}

<<__EntryPoint>>
function test() :mixed{
  dynamic_construct("C");
  dynamic_construct("D");
  dynamic_construct("E");
}
