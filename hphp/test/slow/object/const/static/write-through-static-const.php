<?hh

class A {
  public static int $a = Vector{5};
  <<__Const>>
  public static int $ca = vec[Vector{6}];
}

<<__EntryPoint>>
function test() {
  A::$a[] = 6;
  var_dump(A::$a);

  try {
    A::$ca[0][] = 7;
    echo "FAIL: wrote to static const property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump(A::$ca);
}
