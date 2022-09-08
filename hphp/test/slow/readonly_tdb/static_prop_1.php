<?hh
namespace ROTB\StaticProp1;
class Foo {
  public int $x = 0;
  public static readonly int $sprop = 0;
}

<<__EntryPoint>>
function main(): void {
    $_ = Foo::$sprop; // readonly inferred
}
