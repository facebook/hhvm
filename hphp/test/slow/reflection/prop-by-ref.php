<?hh

class X {
  public $x;
}

function test($x) {
  $p = (new ReflectionProperty('X', 'x'))->getValue($x);
  ++$p;
  var_dump($x->x);
}

<<__EntryPoint>>
function main_prop_by_ref() {
  $x = new X;
  $x->x = 42;

  test($x);
}
