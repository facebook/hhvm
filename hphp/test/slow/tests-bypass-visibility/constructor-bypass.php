<?hh

class WWWTest {}

class PrivateCtor {
  <<__TestsBypassVisibility>>
  private function __construct(private int $val) {}
  public function getVal(): int { return $this->val; }
}

class ProtectedCtor {
  <<__TestsBypassVisibility>>
  protected function __construct(private int $val) {}
  public function getVal(): int { return $this->val; }
}

class PrivateCtorChild extends PrivateCtor {}
class ProtectedCtorChild extends ProtectedCtor {}

class CtorTest extends WWWTest {
  public function test(): void {
    $p = new PrivateCtor(1);
    var_dump($p->getVal());

    $q = new ProtectedCtor(2);
    var_dump($q->getVal());

    $pc = new PrivateCtorChild(3);
    var_dump($pc->getVal());

    $qc = new ProtectedCtorChild(4);
    var_dump($qc->getVal());
  }
}

<<__EntryPoint>>
function main(): void {
  (new CtorTest())->test();
}
