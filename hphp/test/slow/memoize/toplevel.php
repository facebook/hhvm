<?hh

abstract final class TestTopLevelStatics {
  public static $i = 100;
}
<<__Memoize>>
function test_top_level() :mixed{ return TestTopLevelStatics::$i++; }


<<__EntryPoint>>
function main_toplevel() :mixed{
echo test_top_level().' ';
echo test_top_level();
}
