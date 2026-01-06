<?hh

// This one should be selected, directly uses the type structure.
class F1_Test5 extends WWWTest {
  public function test(): void {
    $_ = type_structure(F1_ClassWithTypeConst1::class, 'TBar');
  }
}
