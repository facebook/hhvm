<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum SomeEnum: int {
  FIRST = 2;
  SECOND = 3;
}

enum SecondEnum: string {
  FIRST = "4";
  SECOND = "5";
}

class WithConst {
  const float CFLOAT = 1.2;
  const SomeEnum CENUM = SomeEnum::SECOND;
}

const shape('x' => int, 'y' => SecondEnum) SHAPE =
  shape('x' => 5, 'y' => SecondEnum::SECOND);
const (int, ?(string, float)) OPTION = tuple(7, null);
const array<string, int> ARR = array('a' => 1, 'b' => 2);
const darray<string, int> AGE_RANGE = darray['min' => 21];
const varray<string> MAP_INDEX = varray['MAP_1', 'MAP_2'];

function with_constants(): void {
  $a = WithConst::CFLOAT;
  $b = WithConst::CENUM;
  $c = SHAPE;
  $d = OPTION;
  $e = ARR;
  $f = AGE_RANGE;
  $g = MAP_INDEX;
}
