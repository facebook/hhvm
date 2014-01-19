<?php

function foo20($x) {
  return $x == 0;
}

function foo30($x) {
  return $x === 0;
}

function foo21($x) {
  return $x == 1;
}

function foo31($x) {
  return $x === 1;
}

function foo2n1($x) {
  return $x == -1;
}

function foo3n1($x) {
  return $x === -1;
}

function foo2h($x) {
  return $x == 123456;
}

function foo3h($x) {
  return $x === 123456;
}

var_dump(foo20(0));
var_dump(foo20(1));
var_dump(foo20(-1));
var_dump(foo20(123456));
var_dump(foo20(2));

var_dump(foo21(0));
var_dump(foo21(1));
var_dump(foo21(-1));
var_dump(foo21(123456));
var_dump(foo21(2));

var_dump(foo2n1(0));
var_dump(foo2n1(1));
var_dump(foo2n1(-1));
var_dump(foo2n1(123456));
var_dump(foo2n1(2));

var_dump(foo2h(0));
var_dump(foo2h(1));
var_dump(foo2h(-1));
var_dump(foo2h(123456));
var_dump(foo2h(2));

var_dump(foo30(0));
var_dump(foo30(1));
var_dump(foo30(-1));
var_dump(foo30(123456));
var_dump(foo30(2));

var_dump(foo31(0));
var_dump(foo31(1));
var_dump(foo31(-1));
var_dump(foo31(123456));
var_dump(foo31(2));

var_dump(foo3n1(0));
var_dump(foo3n1(1));
var_dump(foo3n1(-1));
var_dump(foo3n1(123456));
var_dump(foo3n1(2));

var_dump(foo3h(0));
var_dump(foo3h(1));
var_dump(foo3h(-1));
var_dump(foo3h(123456));
var_dump(foo3h(2));

