<?php
function foo() {
  require 'define.inc';
}

function main() {
  define('FOO', 42);
  if (!defined('FOO')) {
    echo 'how can we run phpMyAdmin without defined() working?';
    exit(1);
  }
  define('BAR', "hello");

  define('FOO', 43);
  define('BIZ', array("a", "b", "c"));

  var_dump(FOO);
  var_dump(BAR);

  foo();
  var_dump(BAZ);

  define('BAZ', "baz_a");
  var_dump(BAZ);

  define('HI', strtoupper("hello"));
  echo HI;
  echo HI;
  echo "\n";
}
main();

function bug() {
    return defined('static::SOME_CONST');
}

bug();
