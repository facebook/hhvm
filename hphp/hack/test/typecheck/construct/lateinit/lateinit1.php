<?hh

final class C1 {
  <<__LateInit>>
  private int $prop;
}

final class C2 {
  private int $prop1;
  <<__LateInit>>
  private int $prop2;

  public function __construct(int $x): void {
    $this->prop1 = $x;
  }
}

final class C3 {
  private int $prop1;
  <<__LateInit>>
  private int $prop2;

  public function __construct(int $x): void {
    $this->prop1 = $x;
    $this->prop2 = $x;
  }
}
