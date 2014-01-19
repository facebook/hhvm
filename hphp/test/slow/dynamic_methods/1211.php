<?php

function bar() {
  echo 'bar called';
}
class foo {
  public $functions = array();
  function foo() {
    $function = 'bar';
    print($function);
    print($function());
    $this->functions['test'] = $function;
    print($this->functions['test']());
  }
}
$a = new foo ();
