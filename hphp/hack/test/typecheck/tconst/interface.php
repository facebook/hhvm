<?hh // strict

interface Tabstract {
  abstract const type T as mixed;
}
interface Tconcrete {
  const type T = int;
}

trait Ttrait implements Tconcrete {}

class C implements Tabstract {
  use Ttrait;
}

function test(C::T $x): void {
  test(0);
}
