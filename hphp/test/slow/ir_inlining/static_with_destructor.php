<?php

class A {
  function __destruct() {
    print "destructor\n";
  }

  static function printer() { print "static\n"; }
}

function main() {
  (new A())->printer();
}


<<__EntryPoint>>
function main_static_with_destructor() {
main();
}
