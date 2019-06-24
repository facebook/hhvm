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
    $c->ci++;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $c->i++;

  var_dump($c);
}


<<__EntryPoint>>
function main_incdec() {
test();
}
