<?hh

class A {
  function __construct () { echo "I'm A\n"; }
}

class B extends A {
  function __construct() {
    parent::__construct();
    parent::__construct();
  }
}

<<__EntryPoint>>
function main() :mixed{
  $b = new B;
  echo "Done\n";
}
