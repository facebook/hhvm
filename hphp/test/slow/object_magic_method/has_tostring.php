<?hh

class X {}

class Y extends X {
  function __toString() { return 'Y'; }
}

function test(X $obj, string $s) {
  return $obj == $s;
}

function main() {
  var_dump(test(new X, "X"));
  var_dump(test(new Y, "Y"));
}

main();
