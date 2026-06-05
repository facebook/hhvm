<?hh

class WWWTest {}

trait T1 {
  <<__TestsBypassVisibility>>
  private function from_t1(): void {}
}

trait T2 {
  <<__TestsBypassVisibility>>
  private function from_t2(): void {}
}

class UsesBoth {
  use T1;
  use T2;
}

class MultiTraitTest extends WWWTest {
  public function test(UsesBoth $obj): void {
    $obj->from_t1(); // ok
    $obj->from_t2(); // ok
  }
}
