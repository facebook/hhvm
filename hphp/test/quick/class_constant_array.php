<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

const BOOLCNS = false;

class Cls1 {
  const ARRAY1 = vec[];
  const ARRAY2 = vec[1, 2, 3, 4];
  const ARRAY3 = vec['a', 'b', 'c', 'd'];
  const ARRAY4 = vec[1, vec[false, null], vec[true, 'abc'], 1.23, vec[]];
  const ARRAY5 = vec[vec[], dict[], keyset[]];
  const ARRAY6 = vec[vec[1, 2], dict['abc' => true], keyset['a', 100, 'b']];
  const ARRAY7 = Cls1::ARRAY1;
  const ARRAY8 = vec[Cls1::ARRAY2, Cls1::ARRAY2];
  const ARRAY9 = vec[BOOLCNS ? Cls1::ARRAY2 : Cls1::ARRAY3];
}

class Cls2 {
  const VEC1 = vec[];
  const VEC2 = vec[1, 2, 3, 4];
  const VEC3 = vec['a', 'b', 'c', 'd'];
  const VEC4 = vec[1, vec[false, null], vec[true, 'abc'], 1.23, vec[]];
  const VEC5 = vec[vec[], dict[], keyset[]];
  const VEC6 = vec[vec[1, 2], dict['abc' => true], keyset['a', 100, 'b']];
  const VEC7 = Cls2::VEC1;
  const VEC8 = vec[Cls2::VEC2, Cls2::VEC2];
  const VEC9 = vec[BOOLCNS ? Cls2::VEC2 : Cls2::VEC3];
}

class Cls3 {
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
  const DICT11 = Cls3::DICT1;
  const DICT12 = dict[123 => Cls3::DICT2, 456 => Cls3::DICT2];
  const DICT13 = dict[100 => BOOLCNS ? Cls3::DICT2 : Cls3::DICT3];
}

class Cls4 {
  const KEYSET1 = keyset[];
  const KEYSET2 = keyset[1, 2, 3, 4];
  const KEYSET3 = keyset['a', 'b', 'c'];
  const KEYSET4 = keyset[1, '1', 2, '2'];
  const KEYSET5 = Cls4::KEYSET1;
  const KEYSET6 = keyset[BOOLCNS ? 'a' : 1];
}

<<__EntryPoint>> function main(): void {
var_dump(Cls1::ARRAY1);
var_dump(Cls1::ARRAY2);
var_dump(Cls1::ARRAY3);
var_dump(Cls1::ARRAY4);
var_dump(Cls1::ARRAY5);
var_dump(Cls1::ARRAY6);
var_dump(Cls1::ARRAY7);
var_dump(Cls1::ARRAY8);
var_dump(Cls1::ARRAY9);

var_dump(Cls2::VEC1);
var_dump(Cls2::VEC2);
var_dump(Cls2::VEC3);
var_dump(Cls2::VEC4);
var_dump(Cls2::VEC5);
var_dump(Cls2::VEC6);
var_dump(Cls2::VEC7);
var_dump(Cls2::VEC8);
var_dump(Cls2::VEC9);

var_dump(Cls3::DICT1);
var_dump(Cls3::DICT2);
var_dump(Cls3::DICT3);
var_dump(Cls3::DICT4);
var_dump(Cls3::DICT5);
var_dump(Cls3::DICT6);
var_dump(Cls3::DICT7);
var_dump(Cls3::DICT8);
var_dump(Cls3::DICT9);
var_dump(Cls3::DICT10);
var_dump(Cls3::DICT11);
var_dump(Cls3::DICT12);
var_dump(Cls3::DICT13);

var_dump(Cls4::KEYSET1);
var_dump(Cls4::KEYSET2);
var_dump(Cls4::KEYSET3);
var_dump(Cls4::KEYSET4);
var_dump(Cls4::KEYSET5);
var_dump(Cls4::KEYSET6);
}
