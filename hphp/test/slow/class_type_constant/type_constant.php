<?hh


interface T {
  abstract const type Z;
  abstract const type me as this;
}

abstract class P {
  const type Z = arraykey;
}

final class C extends P implements T {
  const type Z = string;
  const type me = C;

  const type Cint = int;

  // Ensure you can define a constant named type
  const type = 400;
}

final class D {
  const C::Cint type = 200;
}

const type = 123;

<<__EntryPoint>>
function main_type_constant() :mixed{
var_dump(type + 123);
var_dump(type - 123);
var_dump(C::type + D::type);
var_dump(C::type - D::type);
}
