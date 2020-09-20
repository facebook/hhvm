<?hh

class A {
  function __construct() { echo "I'm A"; }
}

class B extends A {
  function __construct() {
    // Incorrect capitalization is intentional
    parent::__constrUct();
  }
}


<<__EntryPoint>>
function main_ctor_compat_case_insensitve() {
$b = new B;
}
