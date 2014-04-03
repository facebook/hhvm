<?php

function arguments_1_defaults($a = 1, $b, $c) {}
function arguments_2_defaults($a, $b = NULL, $c) {}
function arguments_3_defaults($a, $b, $c = array()) {}

function arguments_12_defaults($a = 1, $b = NULL, $c) {}
function arguments_13_defaults($a = 1, $b, $c = array()) {}
function arguments_23_defaults($a, $b = NULL, $c = array()) {}

function arguments_123_defaults($a = 1, $b = NULL, $c = array()) {}

$functions = array(
  'arguments_1_defaults',
  'arguments_2_defaults',
  'arguments_3_defaults',
  'arguments_12_defaults',
  'arguments_13_defaults',
  'arguments_23_defaults',
  'arguments_123_defaults',
);

$results = array();
foreach ($functions as $function) {
  $reflection = new ReflectionFunction($function);

  $params_info = array();
  foreach ($reflection->getParameters() as $param) {
    $params_info[] = $param->isOptional();
  }

  $results[$function] = $params_info;
}

var_dump($results);
