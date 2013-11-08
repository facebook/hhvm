<?php

namespace A {
  class B {}
}

namespace C {
  function main() {
    $name = '\\\\A\B';
    var_dump(new $name);
  }
  main();
}
