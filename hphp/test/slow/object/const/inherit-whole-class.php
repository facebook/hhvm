<?hh

<<__Const>>
class C {
  public int $ci = 0;
  public vec $cv = vec[1];
}

class D extends C {
  public int $di = 2;
  public vec $dv = vec[3];

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);
    $this->ci = 4;
    $this->cv[] = 5;
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
    echo "FAIL: wrote to scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $d->cv[] = 999;
    echo "FAIL: wrote through hack array property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $d->di = 8;
  $d->dv[] = 9;
  $d->lol = 'whut';

  echo "-- at the end --\n";
  var_dump($d);
}
