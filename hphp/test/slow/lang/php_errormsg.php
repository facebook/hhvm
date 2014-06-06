<?php

ini_set('track_errors', true);

error_reporting(-1);
// emit an undefined warning
@var_dump($php_errormsg);
// This will now contain the previous warning
var_dump($php_errormsg);

// With error reporting off we still get the var
error_reporting(0);
strchr();
var_dump($php_errormsg);
error_reporting(-1);

// normal
strpos();
var_dump($php_errormsg);
// silenced
@strstr();
var_dump($php_errormsg);
// I can set the var
$php_errormsg = 42;
var_dump($php_errormsg);

function b() {
  $a = array();
  @$a->b;
  var_dump(isset($php_errormsg));
}

function a() {
  // functions don't get the var in scope
  var_dump(isset($php_errormsg));
  // only builtins set this
  b();
  var_dump(isset($php_errormsg));

  @strlen();
  var_dump($php_errormsg);
  @var_dump((string)array());
  var_dump($php_errormsg);
}
a();
// still 42
var_dump($php_errormsg);

function nosilence() {
  $a = array();
  $a->b;
  var_dump(isset($php_errormsg));
}
nosilence();

// we can turn it off
ini_set('track_errors', false);
@strtr();
// still 42
var_dump($php_errormsg);
