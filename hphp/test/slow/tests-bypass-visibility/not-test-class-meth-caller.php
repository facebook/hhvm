<?hh

class Target {
  <<__TestsBypassVisibility>>
  private function priv(int $x): int { return $x; }
}

class NotATest {
  public function test(Target $obj): void {
    $mc = meth_caller(Target::class, 'priv');
    $mc($obj, 42);
  }
}

<<__EntryPoint>>
function main(): void {
  (new NotATest())->test(new Target());
}
