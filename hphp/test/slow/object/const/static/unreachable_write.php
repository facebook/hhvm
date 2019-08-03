<?hh

class A {
  public static int $a = 55;
  <<__Const>>
  public static int $ca = 5;
}

<<__EntryPoint>>
function test() {
  A::$a = 60;
  var_dump(A::$a);

  var_dump(A::$ca);
}

function dead(){
  A::$ca = 10;
}
