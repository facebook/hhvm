<?php
function t() {

  EvalOrder1514::$t++;
  return true;
}
function f() {

  EvalOrder1514::$f++;
  return false;
}
function i() {

  EvalOrder1514::$i++;
  return 1;
}
function d() {

  EvalOrder1514::$d++;
  return 3.14;
}
function v() {

  EvalOrder1514::$v++;
  return EvalOrder1514::$v;
}
<<__EntryPoint>>
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
  var_dump(EvalOrder1514::$t, EvalOrder1514::$f,           EvalOrder1514::$i, EvalOrder1514::$d,           EvalOrder1514::$v);
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
  var_dump(EvalOrder1514::$t, EvalOrder1514::$f,           EvalOrder1514::$i, EvalOrder1514::$d,           EvalOrder1514::$v);
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
  var_dump(EvalOrder1514::$t, EvalOrder1514::$f,           EvalOrder1514::$i, EvalOrder1514::$d,           EvalOrder1514::$v);
}

abstract final class EvalOrder1514 {
  public static $t = 0;
  public static $f = 0;
  public static $i = 0;
  public static $d = 0;
  public static $v = 'a';
}
