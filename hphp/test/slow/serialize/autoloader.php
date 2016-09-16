<?php

function __random_autoloader($class) {
  var_dump($class);
}
spl_autoload_register('__random_autoloader');

$serialized_str = 'O:1:"A":0:{}';

var_dump(unserialize($serialized_str, array('allowed_classes' => false)));
var_dump(unserialize($serialized_str));
