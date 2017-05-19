<?hh

abstract class Class {

  public abstract function providesDarrayOfBoolToString(
  ): darray<bool, string>;
  public abstract function takesDarrayOrVarrayOfTany(
    darray_or_varray $x,
  ): void;

  public function test() {
    $this->takesDarrayOrVarrayOfTany($this->providesDarrayOfBoolToString());
  }
}
