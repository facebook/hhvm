<?hh
class Foo {
  private $x = Set {}, $y = 1;
}

<<__EntryPoint>>
function main_prop_init_literal_bug() {
$obj = new Foo();
echo "Done\n";
}
