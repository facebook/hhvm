<?php

trait my_trait {
  function my_trait_method() {}
}

class my_base {
  function my_base_method() {}
  function my_override_method() {}
}

class my_class extends my_base {
  function my_method() {}
  use my_trait;
  function my_override_method() {}
  function my_other_override() {}
}

class my_child extends my_class {
  function my_other_override() {}
  function my_child_method() {}
}

$rc = new ReflectionClass('my_child');
var_dump($rc->getMethods());

function __autoload($cls) {
  if ($cls == "MyClass") {
    class MyClass {}
  }
}

interface I {
  function f(MyClass $c);
}

try {
  $r = new ReflectionClass('I');
  foreach ($r->getMethods() as $m) {
    foreach ($m->getParameters() as $p) {
      $p->getClass();
      var_dump($p);
    }
  }
} catch (Exception $e) {
  var_dump($e->getMessage());
}
