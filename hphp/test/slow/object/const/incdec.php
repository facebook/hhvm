<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  public int $i = 1;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    $this->ci++;
    $this->i++;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C();
  echo "-- after constructor completes --\n";
  var_dump($c);

  try {
    $c->ci++;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $c->i++;

  echo "-- at the end --\n";
  var_dump($c);
}
