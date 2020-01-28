<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_keys() {
  var_dump(darray[100 => 'a', true => 'b', 200 => 'c']);
  var_dump(darray[100 => 'a', false => 'b', 200 => 'c']);
  var_dump(darray[100 => 'a', null => 'b', 200 => 'c']);
  var_dump(darray[100 => 'a', 3.14 => 'b', 200 => 'c']);
  var_dump(darray[100 => 'a', STDIN => 'b', 200 => 'c']);
  var_dump(darray[100 => 'a', new stdclass => 'b', 200 => 'c']);
}

function test_cmp() {
  var_dump(varray[1, 2, 3] === vec[1, 2, 3]);
  var_dump(varray[1, 2, 3] !== vec[1, 2, 3]);
  var_dump(varray[1, 2, 3] == vec[1, 2, 3]);
  var_dump(varray[1, 2, 3] != vec[1, 2, 3]);
  var_dump(varray[1, 2, 3] < true);
  var_dump(varray[1, 2, 3] <= true);
  var_dump(varray[1, 2, 3] > true);
  var_dump(varray[1, 2, 3] >= true);
  var_dump(varray[1, 2, 3] <=> true);

  var_dump(vec[1, 2, 3] === varray[1, 2, 3]);
  var_dump(vec[1, 2, 3] !== varray[1, 2, 3]);
  var_dump(vec[1, 2, 3] == varray[1, 2, 3]);
  var_dump(vec[1, 2, 3] != varray[1, 2, 3]);
  var_dump(true < varray[1, 2, 3]);
  var_dump(true <= varray[1, 2, 3]);
  var_dump(true > varray[1, 2, 3]);
  var_dump(true >= varray[1, 2, 3]);
  var_dump(true <=> varray[1, 2, 3]);
}

function test_add() {
  var_dump(varray[1, 2, 3] + varray[1, 2, 3, 4, 5]);
  var_dump(varray[1, 2, 3, 4, 5] + varray[1, 2, 3]);
}

function test_intlike_keys() {
  var_dump(darray['0' => 1]);
  var_dump(darray['0' => 1, 1 => 2, 2 => 3, 3 => 4]);
  var_dump(darray[0 => 1, 1 => 2, '2' => 3, 3 => 4]);
  var_dump(darray[0 => 1, 1 => 2, 2 => 3, '3' => 4]);

  var_dump(darray['10' => 10]);
  var_dump(darray['10' => 10, 20 => 20, 30 => 30]);
  var_dump(darray[10 => 10, '20' => 20, 30 => 30]);
  var_dump(darray[10 => 10, 20 => 20, '30' => 30]);
}


<<__EntryPoint>>
function main_scalars() {
test_keys();
test_cmp();
test_add();
test_intlike_keys();
}
