<?hh

class C {
  <<__Const>>
  public int $ci = 0;
  public int $i = 1;
}

function inc_ref(&$ref) { $ref++; }
function print_with_ref($what, &$ref) { var_dump($what); }

function test_vget() {
  $c = new C();
  var_dump($c);

  try {
    inc_ref(&$c->ci);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  inc_ref(&$c->i);
  print_with_ref($c, &$c->i);
}


<<__EntryPoint>>
function main_bind() {
  test_vget();
}
