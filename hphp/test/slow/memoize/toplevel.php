<?hh

abstract final class TestTopLevelStatics {
  public static $i = 100;
}
<<__Memoize>>
function test_top_level() { return TestTopLevelStatics::$i++; }


<<__EntryPoint>>
function main_toplevel() {
echo test_top_level().' ';
echo test_top_level();
}
