<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__SoftLateInit>> public $p1;

  <<__SoftLateInit>> private $p2;
  <<__SoftLateInit>> public static $p3;
  <<__SoftLateInit>> private static $p4;

  private $p5;
  public static $p6;
  private static $p7;
}

class B extends A {
  <<__SoftLateInit>> public $p1;

  private $p2;
  public static $p3;
  private static $p4;

  <<__SoftLateInit>> private $p5;
  <<__SoftLateInit>> public static $p6;
  <<__SoftLateInit>> private static $p7;
}


<<__EntryPoint>>
function main_redeclare3() {
new B();
echo "DONE\n";
}
