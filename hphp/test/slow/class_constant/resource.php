<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

define('FOO1', xml_parser_create());
define('FOO2', [xml_parser_create(), xml_parser_create()]);

class A {
  const BAR1 = FOO1;
  const BAR2 = [FOO1, FOO1];
  const BAR3 = vec[FOO1, FOO1];
  const BAR4 = dict[1 => FOO1, 'abc' => FOO1];
}

class B {
  const BAR1 = STDIN;
  const BAR2 = [STDIN, STDIN];
  const BAR3 = vec[STDIN, STDIN];
  const BAR4 = dict[1 => STDIN, 'abc' => STDIN];
}

var_dump(FOO1);
var_dump(FOO2);

var_dump(A::BAR1);
var_dump(A::BAR2);
var_dump(A::BAR3);
var_dump(A::BAR4);

var_dump(B::BAR1);
var_dump(B::BAR2);
var_dump(B::BAR3);
var_dump(B::BAR4);
