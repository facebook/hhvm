<?hh

class A {
  const FOO = self::BAR;
  const BAR = self::BAZ;
  const BAZ = self::FOO;
}

class B {
  const FOO = self::BAZ;
  const BAR = self::BAZ;
  const BAZ = self::FOO;
}

class C1 {
  const FOO = C2::BAR;
}

class C2 {
  const BAR = C1::FOO;
}

class D {
  const FOO = self::FOO;
}

class E {
  const FOO1 = self::BAR1 + self::FOO1;
  const BAR1 = self::BAR1 + self::FOO1;

  const FOO2 = self::FOO2 + self::BAR2;
  const BAR2 = self::FOO2 + self::BAR2;
}

class F {
  const FOO = (PHP_VERSION_ID > 10) ? self::BAR : 100;
  const BAR = self::FOO;
}

class G {
  const FOO = (PHP_VERSION_ID < 10) ? self::BAR : 100;
  const BAR = self::FOO;
}

class H1 {
  const FOO = H2::BAR;
}

class I1_NotRecursive {
  const FOO = I2_NotRecursive::BAR;
}

class I1_Recursive {
  const FOO = I2_Recursive::BAR;
}

const BOOLCNS1 = true;
const BOOLCNS2 = false;

class J1 {
  const FOO = BOOLCNS1 ? self::BAR : self::FOO;
  const BAR = 123;
}

class J2 {
  const FOO = BOOLCNS2 ? self::BAR : self::FOO;
  const BAR = 123;
}

class K {
  const FOO = varray[self::BAR];
  const BAR = varray[self::FOO];
}

function test1() { var_dump(A::FOO); }
function test2() { var_dump(A::BAR); }
function test3() { var_dump(A::BAZ); }
function test4() { var_dump(B::FOO); }
function test5() { var_dump(B::BAR); }
function test6() { var_dump(B::BAZ); }
function test7() { var_dump(C1::FOO); }
function test8() { var_dump(C2::BAR); }
function test9() { var_dump(D::FOO); }
function test10() { var_dump(E::FOO1); }
function test11() { var_dump(E::BAR1); }
function test12() { var_dump(E::FOO2); }
function test13() { var_dump(E::BAR2); }
function test14() { var_dump(F::FOO); }
function test15() { var_dump(F::BAR); }
function test16() { var_dump(G::FOO); }
function test17() { var_dump(G::BAR); }
function test18() { var_dump(H1::FOO); }
function test19() {
  var_dump(I1_NotRecursive::FOO);
}
function test20() {
  var_dump(I1_Recursive::FOO);
}
function test21() { var_dump(J1::FOO); }
function test22() { var_dump(J2::FOO); }
function test23() { var_dump(K::FOO); }
function test24() { var_dump(K::BAR); }

const TESTS = vec[
  'test1',
  'test2',
  'test3',
  'test4',
  'test5',
  'test6',
  'test7',
  'test8',
  'test9',
  'test10',
  'test11',
  'test12',
  'test13',
  'test14',
  'test15',
  'test16',
  'test17',
  'test18',
  'test19',
  'test20',
  'test21',
  'test22',
  'test23',
  'test24'
];

<<__EntryPoint>>
function main() {
  $count = __hhvm_intrinsics\apc_fetch_no_check("count");
  if ($count === false) {
    $count = 0;
  }

  if ($count >= count(TESTS)) return;

  $test = TESTS[$count];
  ++$count;
  apc_store("count", $count);

  echo "================ $test ===================\n";
  $test();
}
