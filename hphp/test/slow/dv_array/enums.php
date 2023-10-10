<?hh

enum Day: int {
  SUNDAY = 1;
  MONDAY = 2;
  TUESDAY = 3;
  WEDNESDAY = 4;
  THURSDAY = 5;
  FRIDAY = 6;
  SATURDAY = 7;
}
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

enum Enum3 : arraykey {
  ONE = '1';
  TWO = '2';
  THREE = '3';
}
<<__EntryPoint>>
function main_entry(): void {
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

  echo "============================================\n";

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
