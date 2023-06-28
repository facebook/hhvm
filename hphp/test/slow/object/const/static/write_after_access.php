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
  post_access();
}

function post_access():mixed{
  try {
    A::$ca = 10;
    echo "FAIL: wrote to static const property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
