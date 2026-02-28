<?hh

<<__Const>>
class A {
  public int $denyMutateField;
  <<__Const>>
  public int $permitRedundantConst;
  public static int $allowMutateStatic = 7;

  public function __construct() {
    $this->denyMutateField = 4;
    $this->permitRedundantConst = 27;
  }

  public function fail(): void {
    A::$allowMutateStatic = 23;
    $this->denyMutateField = 42;
  }
}
