<?hh // strict

abstract class C {}
class D extends C {}
class E extends C {}
class UnrelatedC {}

function test(classname<C> $cls): void {
  switch ($cls) {
    case D::class:
      break;
    case E::class:
      break;
    case UnrelatedC::class:
      // This is a limitation of the current approach to checking switch
      // case labels. Ideally we would be able to detect the impossibility
      // of this comparison succeeding.
      break;
    case 1:
      break;
  }
}
