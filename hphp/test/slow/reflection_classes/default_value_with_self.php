<?php
namespace Foo\Bar;
interface A {
  const SCOPE_CONTAINER                = 'container';
  public function set($scope = self::SCOPE_CONTAINER);
}

function main() {
  $rc = new \ReflectionClass("Foo\Bar\A");
  var_dump($rc->isInterface());
  var_dump($rc->getMethod('set')->getParameters()[0]->getDefaultValue());
  var_dump($rc->getMethods()[0]->getParameters()[0]->getDefaultValue());
}

main();
