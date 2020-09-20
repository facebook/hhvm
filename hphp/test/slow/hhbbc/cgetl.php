<?hh

class A {
  function bar() { return 1; }
}

class B extends A {}

function foo($a) {
  return $a->bar();
}

function bar(A $a) {
  return $a->bar();
}

function baz(?A $a) {
  return $a->bar();
}


<<__EntryPoint>>
function main_cgetl() {
foo(new A);
bar(new A);
baz(new A);
}
