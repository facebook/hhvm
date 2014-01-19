<?php

function fsort($a) {
  sort($a);
  return $a;
}

trait T {
  private function bar() {}
  public  function foo() {}
}

class A {
  use T {
    bar as public;
    foo as private;
  }
  function dump() {
    var_dump(fsort(get_class_methods($this)));
  }
}

class B extends A {};

function main() {
  $objA = new A;
  $objB = new B;

  echo "class A\n";
  print_r(fsort(get_class_methods($objA)));

  echo "\nclass B\n";
  print_r(fsort(get_class_methods($objB)));

  echo "\nclass A::dump()\n";
  print_r($objA->dump());

  echo "\nclass B::dump()\n";
  print_r($objB->dump());
}

main();
