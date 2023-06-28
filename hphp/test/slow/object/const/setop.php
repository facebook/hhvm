<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  public int $i = 1;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    $this->ci += 2;
    $this->i += 2;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C();
  echo "-- after constructor completes --\n";
  var_dump($c);

  try {
    $c->ci += 2;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $c->i += 2;

  echo "-- at the end --\n";
  var_dump($c);
}
