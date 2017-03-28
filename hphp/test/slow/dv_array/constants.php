<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

define('VCONST1', varray[1, 2, 3]);
define('DCONST1', darray[100 => 'abc', 'def' => 200]);

const VCONST2 = varray['a', 'b', 'c'];
const DCONST2 = darray['a' => 100, 'b' => 200];

class A {
  const VCONST3 = varray[100, 'value'];
  const DCONST3 = darray[100 => 300, 500 => 800];
  const VCONST4 = varray[STDIN];
  const DCONST4 = darray[100 => STDIN];
}

var_dump(VCONST1);
var_dump(VCONST2);
var_dump(A::VCONST3);
var_dump(A::VCONST4);
var_dump(DCONST1);
var_dump(DCONST2);
var_dump(A::DCONST3);
var_dump(A::DCONST4);
