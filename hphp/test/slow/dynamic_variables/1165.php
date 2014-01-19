<?php

$i = 1;
$j = 2;
$k = 3;
$v = 'i';
var_dump($$v);
$v = 'j';
var_dump($$v);
$v = 'k';
var_dump($$v);
$v = '_FILES';
var_dump($$v);
$v = 'l';
var_dump($$v);
if (true) {
  class A{
    const C = 1;
  }
}
 else {
  class A{
    const C = 1;
  }
}
function foo($p) {
  var_dump($p::C);
}
foo('A');
