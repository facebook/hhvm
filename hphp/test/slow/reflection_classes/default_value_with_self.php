<?php
namespace Foo\Bar {
  interface A {
    const SCOPE_CONTAINER = 'container';
    public function set($a = self::SCOPE_CONTAINER);
  }
  function foo($a = A::SCOPE_CONTAINER) { var_dump($a); }
}

namespace {
  function main() {
    $rc = new ReflectionClass("Foo\\Bar\\A");
    var_dump($rc->isInterface());
    var_dump($rc->getMethod('set')->getParameters()[0]->getDefaultValue());
    var_dump($rc->getMethod('set')->getParameters()[0]->getDefaultValueText());
    var_dump($rc->getMethod('set')->getParameters()[0]->getDefaultValueConstantName());

    $rc = new \ReflectionFunction("Foo\\Bar\\foo");
    var_dump($rc->getParameters()[0]->getDefaultValue());
    var_dump($rc->getParameters()[0]->getDefaultValueText());
    var_dump($rc->getParameters()[0]->getDefaultValueConstantName());
    Foo\Bar\foo();
  }
  main();
}
