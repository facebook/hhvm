<?hh

// This one should not e selected, directly uses a type structure, but not one we care about.
class F1_Test6 extends WWWTest {
  public function test(): void {
    $_ =
      type_structure(F1_ClassWithTypeConst1::class, 'TBarShouldNotBeTraversed');
  }
}
