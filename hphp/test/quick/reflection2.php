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
foreach ($rc->getMethods() as $ix => $meth) {
  echo $ix, ': ', $meth->getName(), "\n";
  echo 'modifiers: ';   var_dump($meth->getModifiers());
  echo 'isFinal: ';     var_dump($meth->isFinal());
  echo 'isAbstract: ';  var_dump($meth->isAbstract());
  echo 'isPublic: ';    var_dump($meth->isPublic());
  echo 'isProtected: '; var_dump($meth->isProtected());
  echo 'isPrivate: ';   var_dump($meth->isPrivate());
  echo 'isStatic: ';    var_dump($meth->isStatic());
  var_dump($meth->getParameters());

  try {
    $proto = $meth->getPrototype();
    echo 'Prototype: ', $proto->class, '::', $proto->name, "\n";
  } catch (ReflectionException $re) {
    echo get_class($re), ': ', $re->getMessage(), "\n";
  }
}

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
