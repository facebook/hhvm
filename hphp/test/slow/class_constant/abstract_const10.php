<?hh

interface I {
  abstract const FOO;
}

interface J {
  const FOO = 1;
}

interface K {
  abstract const FOO;
}

class D implements I, J, K {}


<<__EntryPoint>>
function main_abstract_const10() :mixed{
var_dump(D::FOO);
}
