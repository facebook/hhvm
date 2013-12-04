<?php
class Foo {
  function getThis() {
    return $this;
  }
  function destroyThis() {
    $baz =& $this->getThis();
  }
}
$bar = new Foo();
$bar->destroyThis();
var_dump($bar);
?>