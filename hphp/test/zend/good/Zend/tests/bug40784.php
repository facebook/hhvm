<?hh

class A {
  function __construct () { echo "I'm A\n"; }
}

class B extends A {
  function __construct() {
    parent::__construct();
    parent::__constrUct();
  }
}

<<__EntryPoint>>
function main() {
  $b = new B;
  echo "Done\n";
}
