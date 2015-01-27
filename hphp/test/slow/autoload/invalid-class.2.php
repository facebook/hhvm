<?php

spl_autoload_register(function() {
  var_dump(func_get_args());
});

$name = '-illegal-class';

var_dump(class_exists($name));
new $name();
