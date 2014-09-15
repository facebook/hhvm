<?php

function feature(
  $actual_value,
  $required_value
) {
  return [
    'OK' => $actual_value === $required_value,
    'Value' => $actual_value,
    'Required Value' => $required_value,
  ];
}

print json_encode(
  [
    'hhvm.jit' =>
      feature((bool) ini_get('hhvm.jit'), true),
    'hhvm.jit_pseudomain' =>
      feature((bool) ini_get('hhvm.jit_pseudomain'), true),
    'libpcre has JIT' =>
      feature((bool) ini_get('hhvm.pcre.jit'), true),
    'HHVM build type' =>
      feature(ini_get('hhvm.build_type'), 'Release')
  ],
  JSON_PRETTY_PRINT
)."\n";
