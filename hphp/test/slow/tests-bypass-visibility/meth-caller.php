<?hh
class WWWTest {}
class Target {
  <<__TestsBypassVisibility>>
  private function priv(int $x): int { return $x; }
}
class MethCallerTest extends WWWTest {
  public function test(Target $obj): void {
    $mc = meth_caller(Target::class, 'priv');
    var_dump($mc($obj, 42));
  }
}
<<__EntryPoint>>
function main(): void {
  (new MethCallerTest())->test(new Target());
}
