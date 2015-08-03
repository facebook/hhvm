<?hh // strict

class C {
  const A = 0x10;
  const B = 010;
  const C = 0b101010110;
  const D = 3.141592;
  const E = 1.5e10;
}

function test(): void {
  var_dump(C::A);
  var_dump(C::B);
  var_dump(C::C);
  var_dump(C::D);
  var_dump(C::E);
  var_dump(0x10);
  var_dump(010);
  var_dump(0b101010110);
  var_dump(3.141592);
  var_dump(1.5e10);
}
