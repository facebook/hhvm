<?php

// method bits
$public = 256;
$private = 1024;
$protected = 512;
$abstract = 2;
$final = 4;
$static = 1;

// class bits
$implicit_abstract = 16;
$explicit_abstract = 32;
$class_final = 64;
$implicit_public = 4096;

$args = array(
  $public | $static | $final,
  $public | $implicit_abstract,
  $private | $abstract,
  $public | $protected | $private,
  $protected | $private,
  $public | $private,
  $private | $implicit_public, // okay then php
  $explicit_abstract | $class_final,
  $final | $class_final
);

foreach ($args as $arg) {
  var_dump(Reflection::getModifierNames($arg));
}
