<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

if (__hhvm_intrinsics\launder_value(true)) {
  include 'redefine1.inc';
} else {
  include 'redefine2.inc';
}

class A {
  public darray $p2 = varray[1, 2, 3];
  public array $p3 = varray[1, 2, 3];
  public darray $p4 = varray[1, 2, 3];
  public array $p5 = darray['a' => 1, 'b' => 2];
  public varray $p6 = darray['a' => 1, 'b' => 2];

  public darray $p8 = CONDCONST6;
  public array $p9= CONDCONST4;
  public darray $p10 = CONDCONST4;
  public array $p11 = CONDCONST5;
  public varray $p12 = CONDCONST5;

  public DAlias $p14 = CONDCONST6;
  public AAlias $p15 = CONDCONST4;
  public DAlias $p16 = CONDCONST4;
  public AAlias $p17 = CONDCONST5;
  public VAlias $p18 = CONDCONST5;

  public static darray $s2 = varray[1, 2, 3];
  public static array $s3 = varray[1, 2, 3];
  public static darray $s4 = varray[1, 2, 3];
  public static array $s5 = darray['a' => 1, 'b' => 2];
  public static varray $s6 = darray['a' => 1, 'b' => 2];

  public static darray $s8 = CONDCONST6;
  public static array $s9 = CONDCONST4;
  public static darray $s10 = CONDCONST4;
  public static array $s11 = CONDCONST5;
  public static varray $s12 = CONDCONST5;

  public static DAlias $s14 = CONDCONST6;
  public static AAlias $s15 = CONDCONST4;
  public static DAlias $s16 = CONDCONST4;
  public static AAlias $s17 = CONDCONST5;
  public static VAlias $s18 = CONDCONST5;
}

class B extends A {
  public static function test() {
    new B();
  }
}

B::test();
B::test();
echo "DONE\n";
