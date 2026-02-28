<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  <<__Const>>
  public vec $cv = vec[1];

  public function __construct() {
    echo "-- at parent constructor entry --\n";
    var_dump($this);
    $this->ci = 2;
    $this->cv[] = 3;

    throw new Exception('lol');
  }
}

class D extends C {
  <<__Const>>
  public int $di = 4;
  <<__Const>> 
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

  /* TODO(nzthomas) restore after we support const objects
  try {
    $d->lol = 'whut';
    echo "FAIL: wrote to dynamic property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  */

  echo "-- at the end --\n";
  var_dump($d);
}
