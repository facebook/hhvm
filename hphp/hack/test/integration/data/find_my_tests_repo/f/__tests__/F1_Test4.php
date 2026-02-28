<?hh

// Should be selected, directly using the type structure in helper function.
// (Even though helper is dead).
class F1_Test4 extends WWWTest {

  private static function helper(F1_ClassWithTypeConst1::TBar $bla): void {}

  public function test(): void {
  }
}
