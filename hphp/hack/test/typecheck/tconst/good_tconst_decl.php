<?hh

abstract class A implements IA {
  abstract public type const abs_class_by_class = arraykey;

  public function overriding(): B::vec {
    return Vector {};
  }

  public function testB(B::me::me::me::Bint $x): void {}
}

class B extends A implements IB {
 public type const abs_class_by_class = int;
 public type const me = B;
 public type const Bint = B::hop;
 public type const hop = int;
 public type const vec = ConstVector<int>;

 public function test(): void {
   $this->testB(101);
 }

 public function overriding(): ImmVector<int> {
   return ImmVector {};
 }
 use Impl;
}
