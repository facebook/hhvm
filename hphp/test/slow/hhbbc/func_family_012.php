<?hh

class Base {
  function asd() :mixed{ echo "Base\n"; return 1; }
}

class Obj extends Base {
  function asd() :mixed{ echo "Derived\n"; return "12"; }
}

function foo(Base $x, Obj $y) :mixed{
  $b = true;
  while($b) {
  $x = $b ? $x : $y;
  $foo = $x->asd();
  $b = false;
  }
  var_dump($foo);
}


<<__EntryPoint>>
function main_func_family_012() :mixed{
foo(new Base, new Obj);
foo(new Obj, new Obj);
}
