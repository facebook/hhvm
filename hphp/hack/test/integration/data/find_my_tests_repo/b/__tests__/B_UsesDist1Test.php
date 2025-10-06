<?hh

class B_UsesDist1Test extends WWWTest {
  public function testUsesDist1(B_Def $b): void {
    $b->foo();
  }
}
