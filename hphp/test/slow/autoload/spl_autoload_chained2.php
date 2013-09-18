<?php

spl_autoload_register('my_autoload');

function my_autoload($class) {
  var_dump($class);
  if ($class == 'A')
    $test = class_exists('C');
  include_once ("spl_autoload_" . strtolower($class) . '.inc');
}

$a = new A();
