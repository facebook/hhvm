<?php

function t($c) {
  var_dump($c);
  var_dump(get_class($c));
}

function main() {
  $ext_var = 1;
  $another_ext = new stdClass();

  $closure1 = function() { };
  $closure2 = function() { return 1; };
  $closure3 = function() use ($ext_var) { return $ext_var + 1; };
  $closure4 = function($param) { return $param; };
  $closure5 = function($param) use ($ext_var, $another_ext) { return $param + $ext_var; };
  $closure6 = function($param = 'test') { return $param; };

  t($closure1);
  t($closure2);
  t($closure3);
  t($closure4);
  t($closure5);
  t($closure6);
}

main();
