<?hh

class WWWTest {}

class DiamondBase {
  <<__TestsBypassVisibility>>
  protected function shared(): void {}
}

class Left extends DiamondBase {}
class Right extends DiamondBase {}

// Diamond: both Left and Right inherit shared() from DiamondBase
class DiamondTest extends WWWTest {
  public function test(Left $l, Right $r, DiamondBase $b): void {
    $b->shared();
    $l->shared();
    $r->shared();
  }
}
