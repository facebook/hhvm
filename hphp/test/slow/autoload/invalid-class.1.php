<?php

function __autoload() {
  var_dump(func_get_args());
}

$name = '-illegal-class';

var_dump(class_exists($name));
new $name();
