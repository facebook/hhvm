<?php
namespace A {
  // SORT_REGULAR is a global constant
  const SORT_NUMERIC = 42;
  function a($k = SORT_REGULAR) { var_dump($k); }
  function b($k = SORT_NUMERIC) { var_dump($k); }

  function c($k = \SORT_REGULAR) { var_dump($k); }
  function d($k = \SORT_NUMERIC) { var_dump($k); }

  // A\B\SORT_NUMERIC
  function e($k = B\SORT_NUMERIC) { var_dump($k); }

  function f($k = \B\SORT_NUMERIC) { var_dump($k); }
}

namespace B {
  const SORT_NUMERIC = 'B val';
}
namespace A\B {
  const SORT_NUMERIC = 'A\B val';
}

namespace {
  foreach (array('a', 'b', 'c', 'd', 'e', 'f') as $func) {
    echo "A\\$func reflection:\n";
    $rc = new ReflectionFunction("A\\$func");
    var_dump($rc->getParameters()[0]->getDefaultValue());
    var_dump($rc->getParameters()[0]->getDefaultValueText());
    var_dump($rc->getParameters()[0]->getDefaultValueConstantName());
    echo "\n";
    echo "A\\$func call:\n";
    call_user_func("A\\$func");
    echo "\n";
  }
}
