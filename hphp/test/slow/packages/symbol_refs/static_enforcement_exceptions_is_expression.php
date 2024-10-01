<?hh

module bar;

class Bar {}

<<__EntryPoint>>
public function main_is(): void {
  $m = new Bar();
  $m is Foo;
  echo "No errors\n";
}
