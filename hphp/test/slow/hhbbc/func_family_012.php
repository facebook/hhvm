<?hh

class Base {
  function asd() { echo "Base\n"; return 1; }
}

class Obj extends Base {
  function asd() { echo "Derived\n"; return "12"; }
}

function foo(Base $x, Obj $y) {
  $b = true;
  while($b) {
  $x = $b ? $x : $y;
  $foo = $x->asd();
  $b = false;
  }
  var_dump($foo);
}


<<__EntryPoint>>
function main_func_family_012() {
foo(new Base, new Obj);
foo(new Obj, new Obj);
}
