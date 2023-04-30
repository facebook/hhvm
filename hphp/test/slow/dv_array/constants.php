<?hh

const VCONST2 = varray['a', 'b', 'c'];
const DCONST2 = darray['a' => 100, 'b' => 200];

class A {
  const VCONST3 = varray[100, 'value'];
  const DCONST3 = darray[100 => 300, 500 => 800];
}

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_constants() {

var_dump(VCONST2);
var_dump(A::VCONST3);
var_dump(DCONST2);
var_dump(A::DCONST3);

var_dump(is_varray(VCONST2));
var_dump(is_varray(A::VCONST3));
var_dump(is_darray(DCONST2));
var_dump(is_darray(A::DCONST3));
}
