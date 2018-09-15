<?php

function f1($x) {
  if (count($x) > 0) {
    var_dump($x);
  } else if (count($x[0]) > 0) {
    var_dump($x[0]);
  }
}

function id($x) {
  return $x;
}
function f2($x) {
  if ($x[0]) var_dump(id($x), $x[0]);
}

function f3($x) {
  var_dump($x[0].'/'. $x[1]);
  var_dump($x[0].'/'. $x[1]);
}

function f4($x) {
  $z = @id($x[0]);
  var_dump($z);
  var_dump($x[0]);
}

function f5($x) {
  if ($x[0][id($x[0])-1]) var_dump($x);
}

<<__EntryPoint>>
function main_538() {
f1(array(array(0, 1, 2)));
f1('abc');
f2(null);
f2(array());
f2(array(10));
f3(array('first', 'second'));
f3('AB');
f4(array('e1', 'e2'));
f5(array(0, 1, 2));
}
