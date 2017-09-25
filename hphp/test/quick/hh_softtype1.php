<?hh

class Bar { public function frob() { echo "frob\n"; } }
class Foo {}

function foo(@Bar $x) {
  if ($x) {
    $x->frob();
  }
}

foo(new Foo);
