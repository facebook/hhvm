<?hh

abstract class Class {

  public abstract function providesDarrayOfString(): darray<arraykey, string>;
  public abstract function providesVarrayOfString(): varray<string>;
  public abstract function providesVarrayOfTany(): varray;
  public abstract function providesDarrayOfTany(): darray;
  public abstract function takesDarrayOrVarrayOfString(
    varray_or_darray<string> $x,
  ): void;
  public abstract function takesDarrayOrVarrayOfTany(
    varray_or_darray $x,
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
