<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  public int $i = 1;
}

function test() {
  $c = new C();
  var_dump($c);

  try {
    $c->ci += 2;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $c->i += 2;

  var_dump($c);
}


<<__EntryPoint>>
function main_setop() {
test();
}
