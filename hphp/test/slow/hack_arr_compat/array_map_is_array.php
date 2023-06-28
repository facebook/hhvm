<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_array_map_is_array() :mixed{
var_dump(array_map($x ==> $x, varray[1, 2, 3]));
var_dump(array_filter(varray[1, 2, 3], $x ==> true));
var_dump(array_reduce(varray[1, 2, 3], ($x, $y) ==> $x + $y, 0));

var_dump(array_map($x ==> $x, darray['a' => 1, 'b' => 2, 'c' => 3]));
var_dump(array_filter(darray['a' => 1, 'b' => 2, 'c' => 3], $x ==> true));
var_dump(array_reduce(darray['a' => 1, 'b' => 2, 'c' => 3], ($x, $y) ==> $x + $y, 0));
}
