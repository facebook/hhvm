<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__LateInit>> public $p1;

  <<__LateInit>> private $p2;
  <<__LateInit>> public static $p3;
  <<__LateInit>> private static $p4;

  private $p5;
  public static $p6;
  private static $p7;
}

class B extends A {
  <<__LateInit>> public $p1;

  private $p2;
  public static $p3;
  private static $p4;

  <<__LateInit>> private $p5;
  <<__LateInit>> public static $p6;
  <<__LateInit>> private static $p7;
}


<<__EntryPoint>>
function main_redeclare3() :mixed{
new B();
echo "DONE\n";
}
