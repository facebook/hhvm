<?hh

<<__Const>>
class C {
  public int $ci = 0;
}

class D extends C {
  public int $i = 1;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    $this->ci = 2;
    $this->i = 3;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $d = new D();
  echo "-- after constructor completes --\n";
  var_dump($d);

  try {
    $d->ci = 4;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $d->i = 5;

  echo "-- at the end --\n";
  var_dump($d);
}
