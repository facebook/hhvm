<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  <<__Const>>
  public vec $cv = vec[1];
}

class D extends C {
  public int $di = 2;
}

class E extends D {
  public vec $ev = vec[3];

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);
    $this->ci = 4;
    $this->cv[] = 5;
    $this->di = 6;
    $this->ev[] = 7;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $e = new E();
  echo "-- after constructor completes --\n";
  var_dump($e);

  try {
    $e->ci = 99;
    echo "FAIL: wrote to scalar property\n";
  } catch (Exception $ex) {
    echo $ex->getMessage() . "\n";
  }

  try {
    $e->cv[] = 999;
    echo "FAIL: wrote through hack array property\n";
  } catch (Exception $ex) {
    echo $ex->getMessage() . "\n";
  }

  $e->di = 8;
  $e->ev[] = 9;
  $e->lol = 'whut';

  echo "-- at the end --\n";
  var_dump($e);
}
