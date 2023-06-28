<?hh

class A {
  function bar() :mixed{ return 1; }
}

class B extends A {}

function foo($a) :mixed{
  return $a->bar();
}

function bar(A $a) :mixed{
  return $a->bar();
}

function baz(?A $a) :mixed{
  return $a->bar();
}


<<__EntryPoint>>
function main_cgetl() :mixed{
foo(new A);
bar(new A);
baz(new A);
}
