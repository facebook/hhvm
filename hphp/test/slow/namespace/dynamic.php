<?php

namespace A {
  class B {}
  const C = 'C';
  define(__NAMESPACE__.'\D', 'D');
}

namespace D {
  function main() {
    $name = '\A\B';
    var_dump(new $name);

    $name = '\A\A';
    $name++;
    var_dump(new $name);

    $name = '\\A\B';
    var_dump(new $name);

    var_dump(\A\C);

    $name = '\A\C';
    var_dump(constant($name));

    $name = '\A\B';
    $name++;
    var_dump(constant($name));

    $name = '\\A\C';;
    var_dump(constant($name));

    var_dump(\A\D);

    $name = '\A\D';
    var_dump(constant($name));

    $name = '\A\C';
    $name++;
    var_dump(constant($name));

    $name = '\\A\D';;
    var_dump(constant($name));
  }
  main();
}
