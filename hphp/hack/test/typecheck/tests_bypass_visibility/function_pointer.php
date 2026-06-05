<?hh

class WWWTest {}

class FpTarget {
  <<__TestsBypassVisibility>>
  private static function priv_static(int $x): int { return $x; }

  <<__TestsBypassVisibility>>
  private function priv_inst(int $x): int { return $x; }
}

class FpTest extends WWWTest {
  public function test(FpTarget $obj): void {
    // Function pointer to private static method
    $fp = FpTarget::priv_static<>;
    $_ = $fp(42);

    // meth_caller on private method works in test context.
    $mc = meth_caller(FpTarget::class, 'priv_inst');
    $_ = $mc($obj, 42);
  }
}

class FpNonTest {
  public function test(FpTarget $obj): void {
    $fp = FpTarget::priv_static<>; // error: not in test context
    $_ = $fp(42);

    $mc = meth_caller(FpTarget::class, 'priv_inst'); // error: not in test context
    $_ = $mc($obj, 42);
  }
}
