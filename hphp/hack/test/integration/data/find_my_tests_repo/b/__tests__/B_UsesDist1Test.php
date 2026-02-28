<?hh

class B_UsesDist1Test extends WWWTest {
  public function testUsesDist1(B_DefWrapper $bw): void {
    $bw->b->foo();
  }
}
