<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// ['a', 'b', 'c']
// varray['a', 'b', 'c']
// darray[0 => 'a', 1 => 'b', 2 => 'c']

// [100 => 200, 200 => 300]
// darray[100 => 200, 200 => 300]

// ['a' => 'x', 'b' => 'y', 'c' => 'z']
// darray['a' => 'x', 'b' => 'y', 'c' => 'z']

// Both static and dynamic

const ARR1 = [];
const VARR1 = varray[];
const DARR1 = darray[];

const ARR2 = ['a', 'b', 'c'];
const VARR2 = varray['a', 'b', 'c'];
const DARR2 = darray[0 => 'a', 1 => 'b', 2 => 'c'];

const ARR3 = [100 => 200, 200 => 300];
const DARR3 = darray[100 => 200, 200 => 300];

const ARR4 = ['a' => 'x', 'b' => 'y', 'c' => 'z'];
const DARR4 = ['a' => 'x', 'b' => 'y', 'c' => 'z'];

function test() {
  var_dump(ARR1 == VARR1);
  var_dump(ARR1 === VARR1);
  var_dump(ARR1 != VARR1);
  var_dump(ARR1 !== VARR1);
  var_dump(ARR1 < VARR1);
  var_dump(ARR1 <= VARR1);
  var_dump(ARR1 > VARR1);
  var_dump(ARR1 >= VARR1);
  var_dump(ARR1 <=> VARR1);

  var_dump(VARR1 == ARR1);
  var_dump(VARR1 === ARR1);
  var_dump(VARR1 != ARR1);
  var_dump(VARR1 !== ARR1);
  var_dump(VARR1 < ARR1);
  var_dump(VARR1 <= ARR1);
  var_dump(VARR1 > ARR1);
  var_dump(VARR1 >= ARR1);
  var_dump(VARR1 <=> ARR1);

  var_dump(ARR1 == DARR1);
  var_dump(ARR1 === DARR1);
  var_dump(ARR1 != DARR1);
  var_dump(ARR1 !== DARR1);
  var_dump(ARR1 < DARR1);
  var_dump(ARR1 <= DARR1);
  var_dump(ARR1 > DARR1);
  var_dump(ARR1 >= DARR1);
  var_dump(ARR1 <=> DARR1);

  var_dump(DARR1 == ARR1);
  var_dump(DARR1 === ARR1);
  var_dump(DARR1 != ARR1);
  var_dump(DARR1 !== ARR1);
  var_dump(DARR1 < ARR1);
  var_dump(DARR1 <= ARR1);
  var_dump(DARR1 > ARR1);
  var_dump(DARR1 >= ARR1);
  var_dump(DARR1 <=> ARR1);

  var_dump(VARR1 == DARR1);
  var_dump(VARR1 === DARR1);
  var_dump(VARR1 != DARR1);
  var_dump(VARR1 !== DARR1);
  var_dump(VARR1 < DARR1);
  var_dump(VARR1 <= DARR1);
  var_dump(VARR1 > DARR1);
  var_dump(VARR1 >= DARR1);
  var_dump(VARR1 <=> DARR1);

  var_dump(DARR1 == VARR1);
  var_dump(DARR1 === VARR1);
  var_dump(DARR1 != VARR1);
  var_dump(DARR1 !== VARR1);
  var_dump(DARR1 < VARR1);
  var_dump(DARR1 <= VARR1);
  var_dump(DARR1 > VARR1);
  var_dump(DARR1 >= VARR1);
  var_dump(DARR1 <=> VARR1);

  var_dump(ARR2 == VARR2);
  var_dump(ARR2 === VARR2);
  var_dump(ARR2 != VARR2);
  var_dump(ARR2 !== VARR2);
  var_dump(ARR2 < VARR2);
  var_dump(ARR2 <= VARR2);
  var_dump(ARR2 > VARR2);
  var_dump(ARR2 >= VARR2);
  var_dump(ARR2 <=> VARR2);

  var_dump(VARR2 == ARR2);
  var_dump(VARR2 === ARR2);
  var_dump(VARR2 != ARR2);
  var_dump(VARR2 !== ARR2);
  var_dump(VARR2 < ARR2);
  var_dump(VARR2 <= ARR2);
  var_dump(VARR2 > ARR2);
  var_dump(VARR2 >= ARR2);
  var_dump(VARR2 <=> ARR2);

  var_dump(ARR2 == DARR2);
  var_dump(ARR2 === DARR2);
  var_dump(ARR2 != DARR2);
  var_dump(ARR2 !== DARR2);
  var_dump(ARR2 < DARR2);
  var_dump(ARR2 <= DARR2);
  var_dump(ARR2 > DARR2);
  var_dump(ARR2 >= DARR2);
  var_dump(ARR2 <=> DARR2);

  var_dump(DARR2 == ARR2);
  var_dump(DARR2 === ARR2);
  var_dump(DARR2 != ARR2);
  var_dump(DARR2 !== ARR2);
  var_dump(DARR2 < ARR2);
  var_dump(DARR2 <= ARR2);
  var_dump(DARR2 > ARR2);
  var_dump(DARR2 >= ARR2);
  var_dump(DARR2 <=> ARR2);

  var_dump(VARR2 == DARR2);
  var_dump(VARR2 === DARR2);
  var_dump(VARR2 != DARR2);
  var_dump(VARR2 !== DARR2);
  var_dump(VARR2 < DARR2);
  var_dump(VARR2 <= DARR2);
  var_dump(VARR2 > DARR2);
  var_dump(VARR2 >= DARR2);
  var_dump(VARR2 <=> DARR2);

  var_dump(DARR2 == VARR2);
  var_dump(DARR2 === VARR2);
  var_dump(DARR2 != VARR2);
  var_dump(DARR2 !== VARR2);
  var_dump(DARR2 < VARR2);
  var_dump(DARR2 <= VARR2);
  var_dump(DARR2 > VARR2);
  var_dump(DARR2 >= VARR2);
  var_dump(DARR2 <=> VARR2);

  var_dump(ARR3 == DARR3);
  var_dump(ARR3 === DARR3);
  var_dump(ARR3 != DARR3);
  var_dump(ARR3 !== DARR3);
  var_dump(ARR3 < DARR3);
  var_dump(ARR3 <= DARR3);
  var_dump(ARR3 > DARR3);
  var_dump(ARR3 >= DARR3);
  var_dump(ARR3 <=> DARR3);

  var_dump(DARR3 == ARR3);
  var_dump(DARR3 === ARR3);
  var_dump(DARR3 != ARR3);
  var_dump(DARR3 !== ARR3);
  var_dump(DARR3 < ARR3);
  var_dump(DARR3 <= ARR3);
  var_dump(DARR3 > ARR3);
  var_dump(DARR3 >= ARR3);
  var_dump(DARR3 <=> ARR3);

  var_dump(ARR4 == DARR4);
  var_dump(ARR4 === DARR4);
  var_dump(ARR4 != DARR4);
  var_dump(ARR4 !== DARR4);
  var_dump(ARR4 < DARR4);
  var_dump(ARR4 <= DARR4);
  var_dump(ARR4 > DARR4);
  var_dump(ARR4 >= DARR4);
  var_dump(ARR4 <=> DARR4);

  var_dump(DARR4 == ARR4);
  var_dump(DARR4 === ARR4);
  var_dump(DARR4 != ARR4);
  var_dump(DARR4 !== ARR4);
  var_dump(DARR4 < ARR4);
  var_dump(DARR4 <= ARR4);
  var_dump(DARR4 > ARR4);
  var_dump(DARR4 >= ARR4);
  var_dump(DARR4 <=> ARR4);
}

<<__EntryPoint>>
function main_comparisons() {
test();
}
