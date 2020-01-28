<?hh


class B {
  const BAR1 = STDIN;
  const BAR2 = varray[STDIN, STDIN];
  const BAR3 = vec[STDIN, STDIN];
  const BAR4 = dict[1 => STDIN, 'abc' => STDIN];
}

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_resource() {

var_dump(B::BAR1);
var_dump(B::BAR2);
var_dump(B::BAR3);
var_dump(B::BAR4);
}
