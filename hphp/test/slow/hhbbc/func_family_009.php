<?php

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
  abstract class Middle extends Base {
    abstract function abs();

    function concrete_override() {
      $x = parent::concrete_override();
      return null;
    }
  }
  class Derived extends Middle {
    function abs() {}
  }
} else {
  class Derived extends Base {
    function concrete_override() { return 2; }
  }
}

main(new Base);
main(new Derived);
}
