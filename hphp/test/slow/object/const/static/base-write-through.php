<?hh

class A {
  public static vec<int> $a = vec[5];
  <<__Const>>
  public static vec<vec<int>> $ca = vec[vec[1,2,3]];
}

<<__EntryPoint>>
function test() :mixed{
  A::$a[] = 6;
  var_dump(A::$a);

  try {
    A::$ca[0][0] = 7;
    echo "FAIL: wrote to static const property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump(A::$ca);
}
