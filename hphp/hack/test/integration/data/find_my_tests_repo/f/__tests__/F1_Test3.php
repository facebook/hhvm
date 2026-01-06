<?hh

// Should be selected, directly using the changed typedef in helper function.
// (Even though helper is dead).
class F1_Test3 extends WWWTest {

  private static function helper(F1_TypeAlias1 $bla): void {}

  public function test(): void {}
}
