<?php


function __autoload($name) {
  echo "autoload $name\n";
}

var_dump(get_class_vars('nope'));
