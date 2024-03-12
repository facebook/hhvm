<?hh

class C {

  public function __construct(private string ...$args) {
  }
  public function getNth(int $i): string {
    return $this->args[$i];
  }
  public function put(vec<string> $t): void {
    $this->args = $t;
  }
}
