<?hh

<<__Const>>
class C {
  public int $i = 0;
  public vec $v = vec[1];
}

function test() {
  $c = new C();
  var_dump($c);

  try {
    $c->i = 1;
    echo "FAIL: wrote to scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    $c->v[] = 2;
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

  var_dump($c);
}


<<__EntryPoint>>
function main_basic_whole_class() {
test();
}
