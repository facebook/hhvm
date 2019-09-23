<?hh

trait T {
  public function a() {}
  final public function b() {}

}

class C {
  use T;
  final public function a() = T::a;
  public function b() = T::b;
}

