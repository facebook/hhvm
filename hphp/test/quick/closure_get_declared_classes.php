<?php

$a = function() {};
$b = function() {};
$classes = get_declared_classes();
asort($classes);
foreach ($classes as $class) {
  if (stripos($class, 'Closure') !== FALSE) {
    var_dump($class);
  }
}
