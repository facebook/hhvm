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
    // hphp_set_property is part of the reflection internals, and uses o_set
    hphp_set_property($c, '', 'ci', 2);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  hphp_set_property($c, '', 'i', 3);

  var_dump($c);
}


<<__EntryPoint>>
function main_o_set() {
test();
}
