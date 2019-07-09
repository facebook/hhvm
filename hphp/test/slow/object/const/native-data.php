<?hh

class C extends StringBuffer {
  <<__Const>>
  public int $ci = 0;
  <<__Const>>
  public vec $cv = vec[1];
  public int $i = 2;
  public vec $v = vec[3];

  public function __construct(string $timezone) {
    echo "-- at constructor entry --\n";
    var_dump($this);
    $this->ci = 4;
    $this->cv[] = 5;
    $this->i = 6;
    $this->v[] = 7;
  }
}

<<__EntryPoint>>
function test() {
  $c = new C('America/New_York');
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
  $c->append('hi ');
  $c->append('there');
  var_dump($c->detach());

  echo "-- at the end --\n";
  var_dump($c);
}
