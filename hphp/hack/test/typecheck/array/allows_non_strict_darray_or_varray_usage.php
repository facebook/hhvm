<?hh

abstract class Class {

  public abstract function providesDarrayOfString(): darray<arraykey, string>;
  public abstract function providesVarrayOfString(): varray<string>;
  public abstract function providesVarrayOfTany(): varray;
  public abstract function providesDarrayOfTany(): darray;
  public abstract function takesDarrayOrVarrayOfString(
    darray_or_varray<string> $x,
  ): void;
  public abstract function takesDarrayOrVarrayOfTany(
    darray_or_varray $x,
  ): void;

  public function test() {
    $this->takesDarrayOrVarrayOfString($this->providesDarrayOfString());
    $this->takesDarrayOrVarrayOfString($this->providesVarrayOfString());
    $this->takesDarrayOrVarrayOfString($this->providesDarrayOfTany());
    $this->takesDarrayOrVarrayOfString($this->providesVarrayOfTany());

    $this->takesDarrayOrVarrayOfTany($this->providesDarrayOfString());
    $this->takesDarrayOrVarrayOfTany($this->providesVarrayOfString());
    $this->takesDarrayOrVarrayOfTany($this->providesDarrayOfTany());
    $this->takesDarrayOrVarrayOfTany($this->providesVarrayOfTany());
  }
}
