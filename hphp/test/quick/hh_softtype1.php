<?hh

class Bar { public function frob() :mixed{ echo "frob\n"; } }
class Foo {}

function foo(<<__Soft>> Bar $x) :mixed{
  if ($x) {
    $x->frob();
  }
}
<<__EntryPoint>> function main(): void {
foo(new Foo);
}
