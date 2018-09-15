<?php

class Base {
  function concrete_override() { return "base_or"; }
}

class Derived extends Base {
  function concrete_override() {
    $x = parent::concrete_override();
    $x .= "_derived";
    return $x;
  }
}

function main(Base $b) {
  $x = $b->concrete_override();
  var_dump($x);
}



<<__EntryPoint>>
function main_func_family_007() {
main(new Base);
main(new Derived);
}
