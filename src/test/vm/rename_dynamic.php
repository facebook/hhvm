<?php

function bungle($s) {
  return 314159;
}

function prefix() {
  return "__rename_func_";
}

$stub_name = prefix() . 'strlen';

var_dump(strlen(''));
fb_rename_function('strlen', $stub_name);
fb_rename_function('bungle', 'strlen');
var_dump(strlen(''));

// release stringdata
unset($stub_name);

// try to allocate something in that memory
$stub_name = prefix() . 'hagfish';

$stub_name = prefix() . 'strlen';
fb_rename_function('strlen', 'bungle');
fb_rename_function($stub_name, 'strlen');
var_dump(strlen(''));
