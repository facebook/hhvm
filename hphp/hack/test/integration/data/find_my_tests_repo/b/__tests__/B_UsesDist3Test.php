<?hh

class B_UsesDist3Test extends WWWTest {
  public function testUsesDist3(B_DefWrapper $bw): void {
    // calling the dist2 function means total dist is 3
    B_Uses::usesDist2($bw->b);
  }
}
