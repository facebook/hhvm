<?php

spl_autoload_register('my_autoload');

function my_autoload($class) {
  var_dump($class);
  $test = class_exists($class);
  include_once ("spl_autoload_" . strtolower($class) . '.inc');
}

$a = new A();
