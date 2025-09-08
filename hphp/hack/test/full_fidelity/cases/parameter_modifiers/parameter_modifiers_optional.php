<?hh

class C1 {
  // Optional isn't allowed here anyway, Instead, a default argument should be used
  public function ioio(optional readonly int $x): void {
    echo $x;
  }
}
