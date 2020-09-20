<?hh

class C {
  const dynamic D = 4;
  public int $a = self::D;

  public function f(): void {
    $this->a = self::D;
  }
}
