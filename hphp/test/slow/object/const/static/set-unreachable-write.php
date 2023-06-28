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

  var_dump(A::$ca);
}

function dead():mixed{
  A::$ca = 10;
}
