<?php

function foo($a, $b, &$c) {
  static $num = 0;
  $c[$a][$b] = $num++;
}

function main($foo_str, $cuf, $cufa) {
  $a = array('red' => 4, 'apple' => 12, 'foo' => 1);

  echo "Builtin calls:\n";
  ksort($a);
  sort(&$a);
  var_dump($a);

  echo "Literal calls:\n";
  $b = array('x' => array('y' => 'z'));
  foo('x', 'y', $b);
  foo('a', 'b', &$b);
  var_dump(&$b);

  $c = 'q';
  $d = 'r';

  echo "Plain calls:\n";
  foo(&$c, $d, $b);
  foo(&$c, $d, &$b);
  foo($c, &$d, $b);
  foo($c, &$d, &$b);
  foo(&$c, &$d, $b);
  foo(&$c, &$d, &$b);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "Dynamic calls:\n";
  $foo_str(&$c, $d, $b);
  $foo_str($c, &$d, $b);
  $foo_str($c, $d, &$b);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "CUF/dynamic-CUF calls:\n";
  call_user_func($foo_str, $c, $d, $b);
  $cuf($foo_str, $c, $d, $b);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "CUF + ref calls:\n";
  call_user_func($foo_str, &$c, $d, &$b);
  call_user_func(&$foo_str, $c, &$d, $b);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";


  echo "dynamic-CUF + ref calls:\n";
  call_user_func($foo_str, &$c, $d, &$b);
  call_user_func(&$foo_str, $c, &$d, $b);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  $x = array($c, $d, &$b);
  $y = array(&$c, $d, &$b);
  $z = array($c, &$d, &$b);

  echo "CUFA calls:\n";
  call_user_func_array($foo_str, $x);
  call_user_func_array($foo_str, $y);
  call_user_func_array($foo_str, $z);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "CUFA + ref calls:\n";
  call_user_func_array(&$foo_str, $x);
  call_user_func_array($foo_str, &$y);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "dynamic-CUFA calls:\n";
  $cufa($foo_str, $x);
  $cufa($foo_str, $y);
  $cufa($foo_str, $z);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "dynamic-CUFA + ref calls:\n";
  $cufa(&$foo_str, $x);
  $cufa($foo_str, &$y);
  echo '$b[$c][$d] = $b['.$c.']['.$d.'] = '.$b[$c][$d]."\n";

  echo "Fatal call:\n";
  foo('x', 'y', array());
}

main('foo', 'call_user_func', 'call_user_func_array');
