<?hh

<<__Const>>
class C {
  public int $ci = 0;
  public vec $cv = vec[1];

  public function __construct() {
    echo "-- at parent constructor entry --\n";
    var_dump($this);
    $this->ci = 2;
    $this->cv[] = 3;

    throw new Exception('lol');
  }
}

<<__Const>>
class D extends C {
  public int $di = 4;
  public vec $dv = vec[5];

  public function __construct() {
    try {
      parent::__construct();
    } catch (Exception $_) {}
    echo "-- in child constructor after parent::__construct --\n";
    var_dump($this);
    $this->di = 6;
    $this->dv[] = 7;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $d = new D();
  echo "-- after constructor completes --\n";
  var_dump($d);

  try {
    $d->ci = 99;
    echo "FAIL: wrote to parent's scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $d->di = 999;
    echo "FAIL: wrote to child's scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $d->cv[] = 9999;
    echo "FAIL: wrote through parent's hack array property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $d->dv[] = 99999;
    echo "FAIL: wrote through child's hack array property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $d->lol = 'whut';
    echo "FAIL: wrote to dynamic property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  echo "-- at the end --\n";
  var_dump($d);
}
