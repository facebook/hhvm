<?hh

trait myTrait {
  public static int $a = 55;
  <<__Const>>
  public static int $ca = 5;
}

class A {
  use myTrait;
}

<<__EntryPoint>>
function test() :mixed{
  A::$a = 60;
  var_dump(A::$a);

  try {
    A::$ca = 10;
    echo "FAIL: wrote to static const property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump(A::$ca);
}
