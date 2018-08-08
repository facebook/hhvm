<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static int $s1 = 123;
  public static string $s2 = 'abc';
  public static string $s3 = '123';
  public static int $s4 = 123;
  public static int $s5 = 123;
  public static int $s6 = 123;
}

function test() {
  $cls = __hhvm_intrinsics\launder_value('A');
  $p1 = __hhvm_intrinsics\launder_value('s1');
  $p2 = __hhvm_intrinsics\launder_value('s2');
  $p3 = __hhvm_intrinsics\launder_value('s3');
  $p4 = __hhvm_intrinsics\launder_value('s4');
  $p5 = __hhvm_intrinsics\launder_value('s5');
  $p6 = __hhvm_intrinsics\launder_value('s6');
  $cls::$$p1 = 'abc';
  $cls::$$p2 += 123;
  $cls::$$p3++;
  $cls::$$p4 += 123;

  $v1 = 456;
  $cls::$$p5 =& $v1;
  $v2 =& $cls::$$p6;
}

test();
