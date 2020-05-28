<?hh // partial

class C {
  <<__Rx, __Mutable>>
  public function a(): void {}

  <<__Rx, __MaybeMutable>>
  public function b(): void {}
}
