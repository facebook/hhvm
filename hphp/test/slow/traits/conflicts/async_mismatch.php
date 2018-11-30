<?hh

trait T {
  public async function a() {}
  public async function b() {}
}

class C {
  use T;
  public async function a() = T::a;
  public function b() = T::b;
}
