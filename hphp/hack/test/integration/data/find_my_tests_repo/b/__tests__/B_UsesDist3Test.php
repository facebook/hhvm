<?hh

class B_UsesDist3Test {
  public function testUsesDist3(B_Def $b): void {
    // calling the dist2 function means total dist is 3
    B_Uses::usesDist2($b);
  }
}
