<?hh

class B_UsesDist2Test {
  public function testUsesDist2(B_Def $b): void {
    // calling the dist1 function means total dist is 2
    B_Uses::usesDist1($b);
  }
}
