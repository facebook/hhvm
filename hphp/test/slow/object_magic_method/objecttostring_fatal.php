<?hh

class C {
  public function __toString() {
    echo "__toString called\n";
    return "string";
  }
}

<<__EntryPoint>>
function test() {
  $c = new C();

  echo "==== explicit call ====\n";
  // explicitly calling __toString is fine
  var_dump($c->__toString());

  echo "==== explicit cast ====\n";
  var_dump((string) $c);
}
