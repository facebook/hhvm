<?php

function __autoload($class) {
  echo "exiting\n";
  exit(1);
}

function get_instance($name) {
  $classname = "X$name";
  return new $classname;
}

get_instance('test');
