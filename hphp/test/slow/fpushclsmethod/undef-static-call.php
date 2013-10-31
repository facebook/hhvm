<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Base {
  public static function test() {
    static::clowntown();
  }
}

class Derived1 extends Base {
  public static function clowntown() {
    print "Welcome to Derived1::clowntown()\n";
  }
}

class Derived2 extends Base {
  public static function clowntown() {
    print "Welcome to Derived2::clowntown()\n";
  }
}

function main() {
  Derived1::test();
  Derived2::test();
}

main();
