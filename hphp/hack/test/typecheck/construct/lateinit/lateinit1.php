<?hh // strict

final class C1 {
  <<__LateInit>>
  private string $prop;
}

final class C2 {
  private string $prop1;
  <<__LateInit>>
  private string $prop2;

  public function __construct(string $x): void {
    $this->prop1 = $x;
  }
}

final class C3 {
  private string $prop1;
  <<__LateInit>>
  private string $prop2;

  public function __construct(string $x): void {
    $this->prop1 = $x;
    $this->prop2 = $x;
  }
}
