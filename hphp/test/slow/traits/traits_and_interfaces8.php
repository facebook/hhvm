<?php

interface IParent {
  function f();
}
interface IChild {
  function g();
}

trait TrParent implements IParent {}

trait TrChild implements IChild {
  use TrParent;
}

function main() {
  $rc = new ReflectionClass(TrChild::class);
  var_dump($rc->getInterfaceNames());

  foreach ($rc->getMethods() as $meth) {
    echo $meth->class, '::', $meth->name,
      $meth->isAbstract() ? ' (abstract)' : '', "\n";
  }
}

main();
