<?hh

class C {
  const type FOO = string;
}

function test(C::FOO $foo) {}
