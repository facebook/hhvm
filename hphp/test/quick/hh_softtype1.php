<?hh

class Bar { public function frob() { echo "frob\n"; } }
class Foo {}

function foo(@Bar $x) {
  if ($x) {
    $x->frob();
  }
}
<<__EntryPoint>> function main(): void {
foo(new Foo);
}
