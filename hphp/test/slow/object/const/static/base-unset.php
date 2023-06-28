<?hh

class A {
  public static $a = dict[1 => "white"];
  <<__Const>>
  public static $ca = dict[2 => "green", 10 => "home"];
}

<<__EntryPoint>>
function test() :mixed{
  unset(A::$a[1]);
  var_dump(A::$a);

  try {
    unset(A::$ca[2]);
    echo "FAIL: wrote to static const property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump(A::$ca);
}
