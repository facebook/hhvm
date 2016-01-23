<?php
namespace A {
  interface I {
    const SORT_NUMERIC = 42;
  }
  class Bar {
    function foo($k = I::SORT_NUMERIC) { var_dump($k); }
  }
}
namespace {
  echo "reflection:\n";
  $rc = new ReflectionMethod('A\Bar', 'foo');
  var_dump($rc->getParameters()[0]->getDefaultValue());
  var_dump($rc->getParameters()[0]->getDefaultValueText());
  var_dump($rc->getParameters()[0]->getDefaultValueConstantName());
  echo "call:\n";
  (new A\Bar())->foo();
}
