<?php
class Foo {
  public function nonstaticMethod() {}
}
$foo = new Foo;
spl_autoload_register(array($foo, 'nonstaticMethod'));
$funcs = spl_autoload_functions();
var_dump($funcs);
?>