<?hh
interface I1 {

  public function f(): void;
}

interface I2 extends I1 {
}

class Base {

  public function f(): void {}
}

class Derived extends Base implements I2, I1 {
}
