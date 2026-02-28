<?hh

// This one should not be selected.
class F1_Test9 extends WWWTest {
  public function test(): void {
    // Due to T247845469, this is currently not being selected:
    //
    $_ = F1_TypeStructureForAlias::makeTypeStructure();
  }
}
