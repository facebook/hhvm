<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  <<__Const>>
  public vec $cv = vec[1];
  public int $i = 2;
  public vec $v = vec[3];
}

function test() {
  $c = new C();
  var_dump($c);

  try {
    $c->ci = 9;
    echo "FAIL: wrote to immutable scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $c->cv[] = 99;
    echo "FAIL: wrote through immutable hack array property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $c->i = 4;
  $c->v[] = 5;
  $c->lol = 'whut';

  var_dump($c);
}


<<__EntryPoint>>
function main_basic_individual_prop() {
test();
}
