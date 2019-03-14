<?php

var_dump(HH\autoload_set_paths(
  array(
    'function' => array(),
    'constant' => array(),
    'failure' => 'failure'
  ),
  ''
));
const foo = 0;
const bar = 0;
function failure($kind, $name) {
  var_dump($kind, $name);
}
var_dump(function_exists('foo'));
var_dump(function_exists('bar', false));
var_dump(defined('foo'));
var_dump(defined('bar', false));
var_dump(constant('foo'));
var_dump(constant('bar'));
