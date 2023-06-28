<?hh

class C {
  <<__Const>>
  public $ci = 0;
  public $i = 1;
  <<__Const>>
  public int $ci2 = 2;
  public $i2 = 3;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    unset($this->ci);
    unset($this->i);
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C();
  echo "-- after constructor completes --\n";
  var_dump($c);

  try {
    unset($c->ci2);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  unset($c->i2);

  echo "-- at the end --\n";
  var_dump($c);
}
