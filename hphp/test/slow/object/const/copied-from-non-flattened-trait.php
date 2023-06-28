<?hh

<<__NoFlatten>> trait T {
  <<__Const>>
  public int $ti = 0;
  <<__Const>>
  public vec $tv = vec[1];
}

class C {
  use T;
  public int $ci = 2;
  public vec $cv = vec[3];

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);
    $this->ti = 4;
    $this->tv[] = 5;
    $this->ci = 6;
    $this->cv[] = 7;
  }
}

class D extends C {
  public static function makeParent() :mixed{ return new parent(); }
}

<<__EntryPoint>>
function test() :mixed{
  $c = D::makeParent();
  echo "-- after constructor completes --\n";
  var_dump($c);

  try {
    $c->ti = 99;
    echo "FAIL: wrote to scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $c->tv[] = 999;
    echo "FAIL: wrote through hack array property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $c->ci = 8;
  $c->cv[] = 9;
  $c->lol = 'whut';

  echo "-- at the end --\n";
  var_dump($c);
}
