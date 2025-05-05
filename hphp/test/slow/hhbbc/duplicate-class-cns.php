<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

trait T1 {
  abstract const CONST1 = 123;
}

trait T2 {
  const CONST1 = 987;
}

class C {
  use T1;
  use T2;
}

<<__EntryPoint>>
function main(): void {
  var_dump(C::CONST1);
}
