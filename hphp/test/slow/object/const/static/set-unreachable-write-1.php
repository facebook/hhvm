<?hh

class A {
  public static int $a = 55;
  <<__Const>>
  public static int $ca = 5;
}

<<__EntryPoint>>
function test() :mixed{
  A::$a = 60;
  var_dump(A::$a);
  if (A::$a > 70) {
    A::$ca = 10;
  }
  var_dump(A::$ca);
}
