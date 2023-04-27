<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

final class MyClassWithPostfixMinusMinus {
  public function __postfixMinusMinus(): void {
    throw new Exception();
  }
}

// These should all work without problem
function test(): void {
  ExampleDsl`() ==> {
    $x = 1;
    $x--;
  }`;

  ExampleDsl`(ExampleInt $x) ==> {
    $x--;
  }`;

  ExampleDsl`() ==> {
    $j = 0;
    for ($i = 10; $i > 0; $i--) {
      $j = $j + $i;
    }
    return $j;
  }`;

  ExampleDsl`(MyClassWithPostfixMinusMinus $i) ==> {
    $i--;
  }`;
}
