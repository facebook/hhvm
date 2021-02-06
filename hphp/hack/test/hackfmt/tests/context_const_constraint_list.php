<?hh

abstract class A {
  abstract const ctx C super [output];
}

abstract class B {
  abstract const ctx C super [output] as [output];
}
