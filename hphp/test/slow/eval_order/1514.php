<?php

$GLOBALS['t'] = 0;
$GLOBALS['f'] = 0;
$GLOBALS['i'] = 0;
$GLOBALS['d'] = 0;
$GLOBALS['v'] = 'a';
function t() {
  global $t;
  $t++;
  return true;
}
function f() {
  global $f;
  $f++;
  return false;
}
function i() {
  global $i;
  $i++;
  return 1;
}
function d() {
  global $d;
  $d++;
  return 3.14;
}
function v() {
  global $v;
  $v++;
  return $v;
}
function foo() {
  var_dump(t() + t());
  var_dump(t() + f());
  var_dump(t() + i());
  var_dump(t() + d());
  var_dump(t() + v());
  var_dump(f() + t());
  var_dump(f() + f());
  var_dump(f() + i());
  var_dump(f() + d());
  var_dump(f() + v());
  var_dump(i() + t());
  var_dump(i() + f());
  var_dump(i() + i());
  var_dump(i() + d());
  var_dump(i() + v());
  var_dump(d() + t());
  var_dump(d() + f());
  var_dump(d() + i());
  var_dump(d() + d());
  var_dump(d() + v());
  var_dump(v() + t());
  var_dump(v() + f());
  var_dump(v() + i());
  var_dump(v() + d());
  var_dump(v() + v());
  var_dump($GLOBALS['t'], $GLOBALS['f'],           $GLOBALS['i'], $GLOBALS['d'],           $GLOBALS['v']);
  var_dump(t() - t());
  var_dump(t() - f());
  var_dump(t() - i());
  var_dump(t() - d());
  var_dump(t() - v());
  var_dump(f() - t());
  var_dump(f() - f());
  var_dump(f() - i());
  var_dump(f() - d());
  var_dump(f() - v());
  var_dump(i() - t());
  var_dump(i() - f());
  var_dump(i() - i());
  var_dump(i() - d());
  var_dump(i() - v());
  var_dump(d() - t());
  var_dump(d() - f());
  var_dump(d() - i());
  var_dump(d() - d());
  var_dump(d() - v());
  var_dump(v() - t());
  var_dump(v() - f());
  var_dump(v() - i());
  var_dump(v() - d());
  var_dump(v() - v());
  var_dump($GLOBALS['t'], $GLOBALS['f'],           $GLOBALS['i'], $GLOBALS['d'],           $GLOBALS['v']);
  var_dump(t() * t());
  var_dump(t() * f());
  var_dump(t() * i());
  var_dump(t() * d());
  var_dump(t() * v());
  var_dump(f() * t());
  var_dump(f() * f());
  var_dump(f() * i());
  var_dump(f() * d());
  var_dump(f() * v());
  var_dump(i() * t());
  var_dump(i() * f());
  var_dump(i() * i());
  var_dump(i() * d());
  var_dump(i() * v());
  var_dump(d() * t());
  var_dump(d() * f());
  var_dump(d() * i());
  var_dump(d() * d());
  var_dump(d() * v());
  var_dump(v() * t());
  var_dump(v() * f());
  var_dump(v() * i());
  var_dump(v() * d());
  var_dump(v() * v());
  var_dump($GLOBALS['t'], $GLOBALS['f'],           $GLOBALS['i'], $GLOBALS['d'],           $GLOBALS['v']);
}
foo();
