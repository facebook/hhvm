<?php

class Base {
  function concrete_override() { return false; }
}

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

function main(Base $b) {
  $x = $b->concrete_override();
  var_dump(is_object($x));
}

main(new Base);
main(new Derived);

