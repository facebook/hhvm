<?hh // strict

abstract class C {
  abstract const FOO;
}

interface I {
  const FOO = 1;
}

class D extends C implements I {}


<<__EntryPoint>>
function main_abstract_const9() {
var_dump(D::FOO);
}
