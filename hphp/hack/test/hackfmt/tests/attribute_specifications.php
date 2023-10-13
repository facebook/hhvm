<?hh

class FooIsNotBrokenTest extends FooTest {
  <<
    Override
  >>
  public function foo(): int {
    return 5;
  }

  <<
    DataProvider('provideFooEnvironment'),
    ExpectedException('FooIsBrokenException'),
    ExpectedExceptionCode(
      'ErrorCode::FOO_IS_BROKEN_AND_CANNOT_POSSIBLY_BE_FIXED',
    )
  >>
  public function testFoo(FooEnvironment $env) {
    if ($env->isFooBroken()) {
      throw new FooIsBrokenException();
    }
  }
}

<<Attr1,Attr2 >>class C {
  <<Attr1,Attr2 >>public function f<<<__Soft>>reify T>(<<__Soft>>int $x):<<__Soft>>void {}
}

function f(<<ReallyOverlyLongAttributeNameForTest>>
           int $reallyOverlyLongVariableNameForTest): void {
  <<Attr>> ($x) ==> $x * $x;
  <<AnotherReallyOverlyLongAttributeNameForTest>>
  ($sameXParameterButMuchLongerForTest) ==> multiplyButLonger(
    $sameXParameterButMuchLongerForTest, $sameXParameterButMuchLongerForTest);

  <<Attr>> function (<<__Soft>>int $x):<<__Soft>>void { return $x * $x; };
  <<AnotherReallyOverlyLongAttributeNameForTest>> function (
    <<__Soft>> string $x,
    <<__Soft, YetAnotherEvenMoreOverlyLongAttributeNameForTest>>int $sameXParameterButMuchLongerForTest,
  ):<<__Soft>>void {
    return multiplyButLonger($sameXParameterButMuchLongerForTest, $sameXParameterButMuchLongerForTest);
  };
}
