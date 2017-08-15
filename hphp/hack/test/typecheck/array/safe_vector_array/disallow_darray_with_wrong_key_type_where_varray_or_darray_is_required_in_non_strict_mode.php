<?hh

abstract class Class {

  public abstract function providesDarrayOfBoolToString(
  ): darray<bool, string>;
  public abstract function takesDarrayOrVarrayOfTany(
    varray_or_darray $x,
  ): void;

  public function test() {
    $this->takesDarrayOrVarrayOfTany($this->providesDarrayOfBoolToString());
  }
}
