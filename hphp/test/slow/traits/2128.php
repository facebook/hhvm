<?hh

final class Foo {
  use Bar;
  private static $a = 3;
}
trait Bar {
  private static $a = 3;
}

<<__EntryPoint>>
function main_2128() :mixed{
echo "Done\n";
}
