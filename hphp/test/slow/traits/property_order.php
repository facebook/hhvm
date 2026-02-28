<?hh

trait T1 {
  public $x1, $x2, $x3, $t5, $t6;
}

class U {
  public $u1, $u2;
}

class X extends U {
  use T1;
  public $x1, $x2, $x3, $x4;
}

function main() :mixed{
  $r = new ReflectionClass('X');
  $props = $r->getProperties();
  foreach ($props as $p) {
    var_dump($p->name);
  }
}


<<__EntryPoint>>
function main_property_order() :mixed{
main();
}
