<?php

namespace Foo\Bar {
  const BAZ = 2;

  class C {
    public function func() {
      $x = 40;
      return function ($arg = BAZ) use ($x) { return $x + $arg; };
    }
  }
}

namespace Foo {
  const BAZ = 1;

  function main() {
    var_dump(BAZ);
    var_dump(Bar\BAZ);

    $func = (new Bar\C())->func();
    var_dump($func());

    $rm = new \ReflectionMethod($func, '__invoke');
    var_dump($rm->getName());
    var_dump($rm->getDeclaringClass()->getName());

    $rp = $rm->getParameters()[0];
    var_dump($rp->getName());
    var_dump($rp->getDeclaringFunction()->getName());
    var_dump($rp->getDeclaringFunction()->getDeclaringClass()->getName());
    var_dump($rp->getDeclaringClass()->getName());
    var_dump($rp->getDefaultValue());
    var_dump($rp->getDefaultValueConstantName());
  }

  main();
}
