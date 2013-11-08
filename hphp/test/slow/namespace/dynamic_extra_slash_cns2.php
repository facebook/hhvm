<?php

namespace A {
  define('\A\B', 'B');
}

namespace C {
  function main() {
    var_dump(\A\B);
  }
  main();
}

