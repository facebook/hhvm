<?hh

class X {
  public $x;
}

function test($x) :mixed{
  $p = (new ReflectionProperty('X', 'x'))->getValue($x);
  ++$p;
  var_dump($x->x);
}

<<__EntryPoint>>
function main_prop_by_ref() :mixed{
  $x = new X;
  $x->x = 42;

  test($x);
}
