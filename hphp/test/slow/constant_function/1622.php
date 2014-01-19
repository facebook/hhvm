<?php

var_dump(constant('M_PI'));
$a = 'M_PI';
var_dump(constant($a));
define('FOO', M_PI);
var_dump(constant('FOO'));
define('BAR', php_uname());
var_dump(constant('BAR'));
define(/*|Dynamic|*/'GOO', 1);
var_dump(constant('GOO'));
if (false) {
  define('C', 1);
}
 else {
  define('C', 2);
}
var_dump(constant('C'));
