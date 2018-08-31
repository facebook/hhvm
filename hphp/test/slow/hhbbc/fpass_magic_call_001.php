<?hh

class Foo {
  function __call(string $x, array $args) {
    echo "called\n";
  }
}

class Bar {
  function someUniqueName($x, $y, $z) {
  }
}

function hey() { return new Foo(); }

class A {
  private $foo;
  function go() {
    return hey()->someUniqueName($this->foo);
  }
}


<<__EntryPoint>>
function main_fpass_magic_call_001() {
(new A)->go();
}
