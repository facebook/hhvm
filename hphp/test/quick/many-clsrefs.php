<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Bar {
  public static $prop1;
  public static $prop2;
  public static $prop3;
  public static $prop4;
  public static $prop5;
  public static $prop6;
  public static $prop7;
  public static $prop8;
  public static $prop9;
  public static $prop10;

  static function set($f) :mixed{
    self::$prop1 = self::$prop2 = self::$prop3 =
    self::$prop4 = self::$prop5 = self::$prop6 =
    self::$prop7 = self::$prop8 = self::$prop9 =
    self::$prop10 = $f();
  }

  static function dump() :mixed{
    var_dump(self::$prop1);
    var_dump(self::$prop2);
    var_dump(self::$prop3);
    var_dump(self::$prop4);
    var_dump(self::$prop5);
    var_dump(self::$prop6);
    var_dump(self::$prop7);
    var_dump(self::$prop8);
    var_dump(self::$prop9);
    var_dump(self::$prop10);
  }
}

function main() :mixed{
  Bar::set(() ==> 123);
  Bar::dump();
}
<<__EntryPoint>> function main_entry(): void {
main();
main();
}
