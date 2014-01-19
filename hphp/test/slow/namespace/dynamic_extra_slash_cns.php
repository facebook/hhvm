<?php

namespace A {
  const B = 'B';
}

namespace C {
  function main() {
    $name = '\\\\A\B';
    var_dump(constant($name));
  }
  main();
}
