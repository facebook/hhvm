<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum Enum1 : string {
  ONE = '1';
  TWO = '2';
  THREE = '3';
}

enum Enum2 : int {
  ONE = 1;
  TWO = 2;
  THREE = 3;
}

enum Enum3 : mixed {
  ONE = '1';
  TWO = '2';
  THREE = '3';
}
<<__EntryPoint>> function main(): void {
var_dump(Enum1::getNames());
var_dump(Enum1::getValues());
var_dump(Enum1::isValid('1'));
var_dump(Enum1::isValid(1));
var_dump(Enum1::coerce('1'));
var_dump(Enum1::coerce(1));
var_dump(Enum1::assert('1'));
var_dump(Enum1::assert(1));
var_dump(Enum1::assertAll(vec['1']));
var_dump(Enum1::assertAll(vec[1]));

echo "==============================================\n";

var_dump(Enum2::getNames());
var_dump(Enum2::getValues());
var_dump(Enum2::isValid('1'));
var_dump(Enum2::isValid(1));
var_dump(Enum2::coerce('1'));
var_dump(Enum2::coerce(1));
var_dump(Enum2::assert('1'));
var_dump(Enum2::assert(1));
var_dump(Enum2::assertAll(vec['1']));
var_dump(Enum2::assertAll(vec[1]));

echo "==============================================\n";

var_dump(Enum3::getNames());
var_dump(Enum3::getValues());
var_dump(Enum3::isValid('1'));
var_dump(Enum3::isValid(1));
var_dump(Enum3::coerce('1'));
var_dump(Enum3::coerce(1));
var_dump(Enum3::assert('1'));
var_dump(Enum3::assert(1));
var_dump(Enum3::assertAll(vec['1']));
var_dump(Enum3::assertAll(vec[1]));
}
