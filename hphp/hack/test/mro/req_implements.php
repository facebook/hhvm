<?hh // strict

class A {}
class B {}

interface IA { abstract const type T as A; }
interface IB { abstract const type T as B; }

trait X {
  require implements IB;
  require implements IA;
}

trait Y implements IA {
  use X;
}
