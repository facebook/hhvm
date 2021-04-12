<?hh

interface I1 { function f1(); }
interface I2 extends I1 { function f2(); }
abstract class C1 implements I1 { abstract function f3(); };
abstract class C2 extends C1 implements I2 { abstract function f4(); }

class C3 extends C2 {
  function f1() { echo __FUNCTION__."\n"; }
  function f2() { echo __FUNCTION__."\n"; }
  function f3() { echo __FUNCTION__."\n"; }
  function f4() { echo __FUNCTION__."\n"; }
  function f5() { echo __FUNCTION__."\n"; }
}

<<__EntryPoint>>
function main() {
  $m1 = meth_caller(I1::class, 'f1');
  $m2a = meth_caller(I2::class, 'f1');
  $m2b = meth_caller(I2::class, 'f2');
  $m3a = meth_caller(C1::class, 'f1');
  $m3c = meth_caller(C1::class, 'f3');
  $m4a = meth_caller(C2::class, 'f1');
  $m4b = meth_caller(C2::class, 'f2');
  $m4c = meth_caller(C2::class, 'f3');
  $m4d = meth_caller(C2::class, 'f4');
  $m5a = meth_caller(C3::class, 'f1');
  $m5b = meth_caller(C3::class, 'f2');
  $m5c = meth_caller(C3::class, 'f3');
  $m5d = meth_caller(C3::class, 'f4');
  $m5e = meth_caller(C3::class, 'f5');

  $m1(new C3);
  $m2a(new C3);
  $m2b(new C3);
  $m3a(new C3);
  $m3c(new C3);
  $m4a(new C3);
  $m4b(new C3);
  $m4c(new C3);
  $m4d(new C3);
  $m5a(new C3);
  $m5b(new C3);
  $m5c(new C3);
  $m5d(new C3);
  $m5e(new C3);
}
