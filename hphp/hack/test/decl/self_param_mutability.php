<?hh // partial

class C {
  <<__Pure, __Mutable>>
  public function a(): void {}

  <<__Pure, __MaybeMutable>>
  public function b(): void {}
}
