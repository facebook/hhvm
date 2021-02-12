<?hh
interface A {}
interface B extends A {}

class C0 {

  public function f(): void {
  }
}

class C1 extends C0 {

  public function f(): void {
  }
}

abstract class Class3 extends Class2 {
  use T1;
}

trait T1 {
  require extends Class1;


  final protected function f(
  ): void {
  }
}

abstract class Class2
  extends Class1 {
}

abstract class Class1 implements I0 {

  protected function f(
  ): void {}
}

interface I2 extends I1 {
}

interface I1 extends I0 {
}

interface I0 {
}
