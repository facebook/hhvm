<?php

var_dump(HH\autoload_set_paths(
  array(
    'function' => array(),
    'constant' => array(),
    'failure' => 'failure'
  ),
  ''
));
function failure($kind, $name) {
  if ($kind == 'constant' && $name == 'bar') {
    define('bar', 'baz');
  }
  var_dump($kind, $name);
}

function builtin() {
  var_dump(function_exists('strpos'));
  if (!function_exists('strpos')) {
    echo "dead code\n";
  }
  var_dump(function_exists('strpos', true));
  if (!function_exists('strpos', true)) {
    echo "dead code\n";
  }
  var_dump(function_exists('strpos', false));
  if (!function_exists('strpos', false)) {
    echo "dead code\n";
  }
}
builtin();

var_dump(function_exists('foo'));
var_dump(function_exists('foo', true));
var_dump(function_exists('foo', false));
var_dump(function_exists('bar', false));
var_dump(defined('foo'));
var_dump(defined('bar', false));
var_dump(constant('foo'));
var_dump(constant('bar'));

if (0) {
  function foo() {}
  function foo() {}
  define('foo', 0);
  define('bar', 0);
}
