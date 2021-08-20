<?hh

final class C {
  const ctx C = [write_props];
  const type Tself = C;
}

function f_two<reify T as C>()[T::Tself::C]: void {}

function test()[write_props]: void {
  f_two<C>();
}
