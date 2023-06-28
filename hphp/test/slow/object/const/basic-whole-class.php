<?hh

<<__Const>>
class C {
  public int $i = 0;
  public vec $v = vec[1];

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);
    $this->i = 2;
    $this->v[] = 3;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C();
  echo "-- after constructor completes --\n";
  var_dump($c);

  try {
    $c->i = 99;
    echo "FAIL: wrote to scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $c->v[] = 999;
    echo "FAIL: wrote through hack array property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $c->lol = 'whut';
    echo "FAIL: wrote to dynamic property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  echo "-- at the end --\n";
  var_dump($c);
}
