<?php

function expect_fail($lambda) {
  try {
    $lambda();
  } catch (Exception $e) {
    echo $e->getMessage()." on line ".$e->getLine()."\n";
    return;
  }
  throw new Exception('Call failed to fail.');
}

abstract final class FooStatics {
  public static $num = 0;
}

function foo($a, $b, &$c) {
  $c[$a][$b] = FooStatics::$num++;
}

function main($foo, $cuf, $cufa) {
  $a = array('red' => 4, 'apple' => 12, 'foo' => 1);

  echo "Builtin calls:\n";
  expect_fail(function() use(&$a) { ksort($a); });
  var_dump($a);
  sort(&$a);
  var_dump($a);

  echo "Literal calls:\n";
  $b = array('x' => array('y' => 'z'));
  expect_fail(function() use(&$b) { foo('x', 'y', $b); });
  var_dump($b);
  foo('a', 'b', &$b);
  var_dump($b);

  $c = 'q';
  $d = 'r';

  echo "Plain calls:\n";
  expect_fail(function() use(&$c, &$d, &$b) { foo($c, $d, $b); });
  foo($c, $d, &$b);
  expect_fail(function() use(&$c, &$d, &$b) { foo($c, &$d, $b); });
  expect_fail(function() use(&$c, &$d, &$b) { foo($c, &$d, &$b); });
  expect_fail(function() use(&$c, &$d, &$b) { foo(&$c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b) { foo(&$c, $d, &$b); });
  expect_fail(function() use(&$c, &$d, &$b) { foo(&$c, &$d, $b); });
  expect_fail(function() use(&$c, &$d, &$b) { foo(&$c, &$d, &$b); });
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "Dynamic calls:\n";
  expect_fail(function() use(&$c, &$d, &$b, $foo) { $foo(&$c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo) { $foo($c, &$d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo) { $foo($c, $d, $b); });
  $foo($c, $d, &$b);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "CUF/dynamic-CUF calls:\n";
  expect_fail(function() use(&$c, &$d, &$b, $foo) { call_user_func($foo, $c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo, $cuf) { $cuf($foo, $c, $d, $b); });
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "CUF + ref calls:\n";
  expect_fail(function() use(&$c, &$d, &$b, $foo) { call_user_func($foo, $c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo) { call_user_func(&$foo, $c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo) { call_user_func($foo, &$c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo) { call_user_func($foo, $c, &$d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo) { call_user_func($foo, $c, $d, &$b); });
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "dynamic-CUF + ref calls:\n";
  expect_fail(function() use(&$c, &$d, &$b, $foo, $cuf) { $cuf($foo, $c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo, $cuf) { $cuf(&$foo, $c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo, $cuf) { $cuf($foo, &$c, $d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo, $cuf) { $cuf($foo, $c, &$d, $b); });
  expect_fail(function() use(&$c, &$d, &$b, $foo, $cuf) { $cuf($foo, $c, $d, &$b); });
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  $x = array($c, $d, &$b);
  $y = array(&$c, $d, &$b);
  $z = array($c, &$d, &$b);

  echo "CUFA calls:\n";
  expect_fail(() ==> call_user_func_array($foo, $x));
  expect_fail(() ==> call_user_func_array($foo, $y));
  expect_fail(() ==> call_user_func_array($foo, $z));
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "CUFA + ref calls:\n";
  expect_fail(function() use(&$foo, &$x) { call_user_func_array(&$foo, $x); });
  expect_fail(function() use(&$foo, &$y) { call_user_func_array($foo, &$y); });
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "dynamic-CUFA calls:\n";
  expect_fail(() ==> $cufa($foo, $x));
  expect_fail(() ==> $cufa($foo, $y));
  expect_fail(() ==> $cufa($foo, $z));
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "dynamic-CUFA + ref calls:\n";
  expect_fail(function() use(&$foo, &$x, $cufa) { $cufa(&$foo, $x); });
  expect_fail(function() use(&$foo, &$y, $cufa) { $cufa($foo, &$y); });
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "fb_intercept:\n";
  if (!ini_get('hhvm.repo.authoritative')) intercept();

  echo "Fatal call:\n";
  expect_fail(() ==> foo('x', 'y', array()));
}

function foo2($x, $y, $z, $t, $q) {
  var_dump("foo", $x);
}

function bar($name, $obj, $params, $data, &$done) {
  var_dump("bar", func_get_args());
}

function intercept() {
  fb_intercept('foo2', 'bar', null);
  foo2(1, 2, 3, 4, 5);
}


<<__EntryPoint>>
function main_ref_annotate_strict() {
main('foo', 'call_user_func', 'call_user_func_array');
}
