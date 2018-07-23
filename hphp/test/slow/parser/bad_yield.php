<?hh

class C {
  function __construct() {
    function foo() {}
    yield foo();
  }
}

echo "Done\n";
