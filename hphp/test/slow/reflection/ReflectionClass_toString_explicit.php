<?hh

// Explicit ReflectionClass::__toString() no longer throws: it stringifies its
// property/method sub-objects via explicit __toString(), not a (string) cast.
class ToStringSubject {
  public int $pub = 1;
  protected string $prot = "x";
  private float $priv = 2.0;
  public static int $spub = 3;
  protected static string $sprot = "y";
  public function m(int $a): void {}
  private function pm(): void {}
  public static function sm(): void {}
}

<<__EntryPoint>>
function main(): void {
  echo (new ReflectionClass('ToStringSubject'))->__toString();
}
