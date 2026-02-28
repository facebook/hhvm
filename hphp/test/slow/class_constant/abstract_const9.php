<?hh

abstract class C {
  abstract const FOO;
}

interface I {
  const FOO = 1;
}

class D extends C implements I {}


<<__EntryPoint>>
function main_abstract_const9() :mixed{
var_dump(D::FOO);
}
