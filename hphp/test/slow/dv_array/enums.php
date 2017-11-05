<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum Day: int {
  SUNDAY = 1;
  MONDAY = 2;
  TUESDAY = 3;
  WEDNESDAY = 4;
  THURSDAY = 5;
  FRIDAY = 6;
  SATURDAY = 7;
};

$x = Day::getNames();
var_dump($x);
var_dump(is_array($x));
var_dump(is_varray($x));
var_dump(is_darray($x));

echo "============================================\n";

$x = Day::getValues();
var_dump($x);
var_dump(is_array($x));
var_dump(is_varray($x));
var_dump(is_darray($x));
