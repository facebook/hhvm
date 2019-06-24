<?hh

<<__Const>>
class C {
  public int $cv = Vector{0};
  public int $cvv = vec[Vector{1}];
}

function test() {
  $c = new C();
  var_dump($c);

  try {
    $c->cv[] = 2;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $lv = $c->cv;
  $lv[] = 3;

  try {
    $c->cvv[0][] = 4;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $lv = $c->cvv[0];
  $lv[] = 5;

  var_dump($c);
}


<<__EntryPoint>>
function main_dim_for_write() {
test();
}
