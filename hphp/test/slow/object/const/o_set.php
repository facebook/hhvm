<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  public int $i = 1;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    // hphp_set_property is part of the reflection internals, and uses o_set
    hphp_set_property($this, '', 'ci', 2);
    hphp_set_property($this, '', 'i', 3);
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C();
  echo "-- after constructor completes --\n";
  var_dump($c);

  try {
    hphp_set_property($c, '', 'ci', 4);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  hphp_set_property($c, '', 'i', 5);

  echo "-- at the end --\n";
  var_dump($c);
}
