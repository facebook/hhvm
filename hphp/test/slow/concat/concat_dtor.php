<?hh

function foo($x, $y) {
  return $x.$y;
}

class dtor {
  private $i;
  function __construct($i) { $this->i = $i; }
  function __toString() { echo "toString: $this->i\n"; return "a"; }
}

function go() {
  foo(new dtor(1), new dtor(2));
  foo(new dtor(3), new dtor(4));
  foo(new dtor(5), new dtor(6));
  foo(new dtor(7), new dtor(8));
}


<<__EntryPoint>>
function main_concat_dtor() {
  go();
  var_dump(hh\objprof_get_data());
}
