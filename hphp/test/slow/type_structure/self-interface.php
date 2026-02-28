<?hh

class U1 {}
class U2 {}

interface I {
  abstract const type U;
  const type T = self::U;
}

class A implements I {
  const type U = U1;
}

class B implements I {
  const type U = U2;
}

class B2 extends B {
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(type_structure('B2', 'T'));
}
