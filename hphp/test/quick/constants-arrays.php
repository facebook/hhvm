<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

const BOOLCNS = false;

class Cls {
  const INTCNS = 123;
}

const ARRAY1 = vec[];
const ARRAY2 = vec[1, 2, 3, 4];
const ARRAY3 = vec['a', 'b', 'c', 'd'];
const ARRAY4 = vec[1, vec[false, null], vec[true, 'abc'], 1.23, vec[]];
const ARRAY5 = vec[vec[], dict[], keyset[]];
const ARRAY6 = vec[vec[1, 2], dict['abc' => true], keyset['a', 100, 'b']];
const ARRAY7 = ARRAY1;
const ARRAY8 = vec[ARRAY2, ARRAY2];
const ARRAY9 = vec[BOOLCNS ? ARRAY2 : ARRAY3];
const ARRAY10 = vec[Cls::INTCNS];

const VEC1 = vec[];
const VEC2 = vec[1, 2, 3, 4];
const VEC3 = vec['a', 'b', 'c', 'd'];
const VEC4 = vec[1, vec[false, null], vec[true, 'abc'], 1.23, vec[]];
const VEC5 = vec[vec[], dict[], keyset[]];
const VEC6 = vec[vec[1, 2], dict['abc' => true], keyset['a', 100, 'b']];
const VEC7 = VEC1;
const VEC8 = vec[VEC2, VEC2];
const VEC9 = vec[BOOLCNS ? VEC2 : VEC3];
const VEC10 = vec[Cls::INTCNS];

const DICT1 = dict[];
const DICT2 = dict[100 => 1, 200 => 2, 300 => 3, 400 => 4];
const DICT3 = dict['key1' => 'a', 'key2' => 'b', 'key3' => 'c', 'key4' => 'd'];
const DICT4 = dict['key1' => 500, 'key2' => 800];
const DICT5 = dict[100 => 'abc', 200 => 'def'];
const DICT6 = dict['100' => 'abc', '200' => 'def', 100 => 'ghi', 200 => 'jkl'];
const DICT7 = dict[0 => 1,
                   1 => dict['a' => false, 5 => null],
                   2 => dict[10 => true, 'z' => 'abc'],
                   3 => 1.23,
                   4 => dict[]];
const DICT8 = dict['100' => 5, 100 => 'abc', 1 => dict[123 => 'abc', '123' => 10]];
const DICT9 = dict[100 => vec[], 200 => vec[], 300 => keyset[]];
const DICT10 = dict[100 => vec[1, 2], 200 => vec['abc'], 300 => keyset['a', 100, 'b']];
const DICT11 = DICT1;
const DICT12 = dict[123 => DICT2, 456 => DICT2];
const DICT13 = dict[100 => BOOLCNS ? DICT2 : DICT3];
const DICT14 = dict['abc' => Cls::INTCNS];

const KEYSET1 = keyset[];
const KEYSET2 = keyset[1, 2, 3, 4];
const KEYSET3 = keyset['a', 'b', 'c'];
const KEYSET4 = keyset[1, '1', 2, '2'];
const KEYSET5 = KEYSET1;
const KEYSET6 = keyset[BOOLCNS ? 'a' : 1];
const KEYSET7 = keyset[Cls::INTCNS];

<<__EntryPoint>> function main(): void {
var_dump(ARRAY1);
var_dump(ARRAY2);
var_dump(ARRAY3);
var_dump(ARRAY4);
var_dump(ARRAY5);
var_dump(ARRAY6);
var_dump(ARRAY7);
var_dump(ARRAY8);
var_dump(ARRAY9);
var_dump(ARRAY10);

var_dump(VEC1);
var_dump(VEC2);
var_dump(VEC3);
var_dump(VEC4);
var_dump(VEC5);
var_dump(VEC6);
var_dump(VEC7);
var_dump(VEC8);
var_dump(VEC9);
var_dump(VEC10);

var_dump(DICT1);
var_dump(DICT2);
var_dump(DICT3);
var_dump(DICT4);
var_dump(DICT5);
var_dump(DICT6);
var_dump(DICT7);
var_dump(DICT8);
var_dump(DICT9);
var_dump(DICT10);
var_dump(DICT11);
var_dump(DICT12);
var_dump(DICT13);
var_dump(DICT14);

var_dump(KEYSET1);
var_dump(KEYSET2);
var_dump(KEYSET3);
var_dump(KEYSET4);
var_dump(KEYSET5);
var_dump(KEYSET6);
var_dump(KEYSET7);
}
