<?php

namespace A {
  function f() { return 'A/f'; }
}

namespace C {
  function main() {
    $name = '\\\\A\f';
    var_dump($name());
  }
  main();
}
