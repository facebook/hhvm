<?php

$direct_ini_file = "include_path_parse_direct_dir.ini";
$nested_ini_file = "include_path_parse_nested_dir.ini";
$nested_dir = "ini_files";

$full_direct_ini_file = __DIR__.'/'.$direct_ini_file;
$full_nested_ini_file = __DIR__.'/'.$nested_dir.'/'.$nested_ini_file;

$initial_cwd = getcwd();

// Empty, fail.
var_dump(parse_ini_file(''));

// Direct: relative (from containing file path).
var_dump(parse_ini_file($direct_ini_file));

// Nested: relative (containing file path, fail).
var_dump(parse_ini_file($nested_ini_file));

// Direct: relative from CWD path.
chdir(__DIR__);
var_dump(parse_ini_file($direct_ini_file));

// Absolute paths.
chdir($initial_cwd);
var_dump(parse_ini_file($full_direct_ini_file));
var_dump(parse_ini_file($full_nested_ini_file));

// Nested: relative from include path.
set_include_path(__DIR__.'/'.$nested_dir);
var_dump(parse_ini_file($nested_ini_file));
