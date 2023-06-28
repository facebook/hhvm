<?hh

<<__Const>>
class C {
  public Vector<int> $cv = Vector{0};
  public vec<Vector<int>> $cvv = vec[Vector{1}];

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);

    $this->cv[] = 2;
    $this->cvv[0][] = 3;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C();
  echo "-- after constructor completes --\n";
  var_dump($c);

  try {
    $c->cv[] = 4;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $lv = $c->cv;
  $lv[] = 5;

  try {
    $c->cvv[0][] = 6;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $lv = $c->cvv[0];
  $lv[] = 7;

  echo "-- at the end --\n";
  var_dump($c);
}
