<?php

class Base {
  function concrete_override() { return "base_or"; }
}

abstract class Derived extends Base {
  abstract function abs();

  function concrete_override() {
    $x = parent::concrete_override();
    $x .= "_derived";
    return $x;
  }
}

class MoreDerived extends Derived {
  function abs() {}
}

function main(Base $b) {
  $x = $b->concrete_override();
  var_dump($x);
}

main(new Base);
main(new MoreDerived);

