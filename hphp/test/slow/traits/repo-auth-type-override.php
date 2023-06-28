<?hh

class B { static public $x = 1; }
trait T { static public $x = 1; }
class C extends B { use T; }

<<__NoFlatten>> trait T2 { static public $x = 1; }
class C2 extends B { use T2; }

class C3 extends B {
  use T2;
  static public $x = 1;
}

class B2 { static public $x = 1; }
class C4 extends B2 {}

trait T3 { static public $x = 1; }
class C5 { use T3; }

<<__NoFlatten>> trait T4 { static public $x = 1; }
class C6 { use T4; }

function test1() :mixed{
  echo "================= test1 ================\n";
  var_dump(B::$x);
  var_dump(T::$x);
  var_dump(C::$x);
  C::$x = 'abc';
  var_dump(B::$x);
  var_dump(T::$x);
  var_dump(C::$x);
}

function test2() :mixed{
  echo "================= test2 ================\n";
  var_dump(B::$x);
  var_dump(T2::$x);
  var_dump(C2::$x);
  C2::$x = 'abc';
  var_dump(B::$x);
  var_dump(T2::$x);
  var_dump(C2::$x);
}

function test3() :mixed{
  echo "================= test3 ================\n";
  var_dump(B::$x);
  var_dump(T::$x);
  var_dump(C3::$x);
  C3::$x = 'abc';
  var_dump(B::$x);
  var_dump(T::$x);
  var_dump(C3::$x);
}

function test4() :mixed{
  echo "================= test4 ================\n";
  var_dump(B2::$x);
  var_dump(C4::$x);
  C4::$x = 'abc';
  var_dump(B2::$x);
  var_dump(C4::$x);
}

function test5() :mixed{
  echo "================= test5 ================\n";
  var_dump(T3::$x);
  var_dump(C5::$x);
  C5::$x = 'abc';
  var_dump(T3::$x);
  var_dump(C5::$x);
}

function test6() :mixed{
  echo "================= test6 ================\n";
  var_dump(T4::$x);
  var_dump(C6::$x);
  C6::$x = 'abc';
  var_dump(T4::$x);
  var_dump(C6::$x);
}

<<__EntryPoint>>
function main() :mixed{
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
}
