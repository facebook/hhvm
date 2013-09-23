<?php

spl_autoload_register('my_autoload');

function my_autoload($class) {
  return class_exists($class, true);
}

$a = new A();
