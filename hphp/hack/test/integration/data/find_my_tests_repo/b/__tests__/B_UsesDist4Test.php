<?hh

class B_UsesDist4Test extends WWWTest {
  public function testUsesDist4(B_DefWrapper $bw): void {
    // calling the dist3 function means total dist is 4
    B_Uses::usesDist3($bw->b);
  }
}
