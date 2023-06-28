<?hh

interface I {
  const v = 42;
}
class X implements I {}
class A extends X {
  const v = 43;
}

class B extends A {}
class C extends A { const v = 44; }


<<__EntryPoint>>
function main_interface_override() :mixed{
var_dump(X::v, A::v, B::v, C::v);
}
