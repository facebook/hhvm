<?hh
class WWWTest {}
class Target {
  <<__TestsBypassVisibility>>
  private static function priv(int $x): int { return $x; }

  <<__TestsBypassVisibility>>
  protected static function prot(int $x): int { return $x + 1; }
}
class FuncPtrTest extends WWWTest {
  public function test(): void {
    $f = Target::priv<>;
    var_dump($f(42));
    $g = Target::prot<>;
    var_dump($g(52));
  }
}
<<__EntryPoint>>
function main(): void {
  (new FuncPtrTest())->test();
}
