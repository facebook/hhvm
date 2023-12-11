<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_array_map_is_array() :mixed{
var_dump(array_map($x ==> $x, vec[1, 2, 3]));
var_dump(array_filter(vec[1, 2, 3], $x ==> true));
var_dump(array_reduce(vec[1, 2, 3], ($x, $y) ==> $x + $y, 0));

var_dump(array_map($x ==> $x, dict['a' => 1, 'b' => 2, 'c' => 3]));
var_dump(array_filter(dict['a' => 1, 'b' => 2, 'c' => 3], $x ==> true));
var_dump(array_reduce(dict['a' => 1, 'b' => 2, 'c' => 3], ($x, $y) ==> $x + $y, 0));
}
