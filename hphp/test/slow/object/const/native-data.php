<?hh

class C extends __hhvm_intrinsics\ExtensibleNewableClassWithNativeData {
  <<__Const>>
  public int $ci = 0;
  <<__Const>>
  public vec $cv = vec[1];
  public int $i = 2;
  public vec $v = vec[3];

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);
    $this->ci = 4;
    $this->cv[] = 5;
    $this->i = 6;
    $this->v[] = 7;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C();
  echo "-- after constructor completes --\n";
  var_dump($c);

  try {
    $c->ci = 99;
    echo "FAIL: wrote to const scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $c->cv[] = 999;
    echo "FAIL: wrote through const hack array property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $c->i = 8;
  $c->v[] = 9;
  $c->lol = 'whut';

  echo "-- test behavior of inherited-from native class --\n";
  $c->setDummyValue(42);
  var_dump($c->getDumyValue());

  echo "-- at the end --\n";
  var_dump($c);
}
