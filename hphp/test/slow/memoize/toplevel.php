<?hh

abstract final class TestTopLevelStatics {
  public static $i = 100;
}
<<__Memoize>>
function test_top_level() :mixed{
  $__lval_tmp_0 = TestTopLevelStatics::$i;
  TestTopLevelStatics::$i++;
  return $__lval_tmp_0;
}


<<__EntryPoint>>
function main_toplevel() :mixed{
echo test_top_level().' ';
echo test_top_level();
}
