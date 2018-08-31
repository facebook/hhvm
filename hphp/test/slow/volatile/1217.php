<?php

function foo() {
  if (!defined('Auth_OpenID_NO_MATH_SUPPORT')) {
    define('Auth_OpenID_NO_MATH_SUPPORT', true);
  }
}
function bar() {
  return defined('Auth_OpenID_NO_MATH_SUPPORT');
}

<<__EntryPoint>>
function main_1217() {
if (defined('M_PI')) {
  var_dump(bar());
  foo();
  var_dump(bar());
}
}
