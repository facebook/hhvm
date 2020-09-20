<?hh

class Base {
  function concrete_override() { return false; }
}

function main(Base $b) {
  $x = $b->concrete_override();
  var_dump(is_object($x));
}



<<__EntryPoint>>
function main_func_family_009() {
if (mt_rand() > 100) {
  include 'func_family_009-1.inc';
} else {
  include 'func_family_009-2.inc';
}

main(new Base);
main(new Derived);
}
