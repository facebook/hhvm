<?hh

class CloneUsageTest {

  public function test(): void {
    $x = Vector {};
    $y = clone $x;
    $y->add(3);
  }

}
