<?php

var_dump(fb_autoload_map(           array('function' => array(),                 'constant' => array(),                 'failure' => 'failure'),           ''));
function failure($kind, $name) {
  if ($kind == 'constant' && $name == 'bar') define('bar', 'baz');
  var_dump($kind, $name);
}
var_dump(function_exists('foo'));
var_dump(function_exists('bar', false));
var_dump(defined('foo'));
var_dump(defined('bar', false));
var_dump(constant('foo'));
var_dump(constant('bar'));
if (0) {
  function foo() {
}
  function foo() {
}
  define('foo', 0);
  define('bar', 0);
}
