<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

final class MyClassWithPostfixPlusPlus {
  public function __postfixPlusPlus(): MyClassWithPostfixPlusPlus {
    throw new Exception();
  }
}

// These should all work without problem
function test(): void {
  ExampleDsl`() ==> {
    $x = 1;
    $x++;
  }`;

  ExampleDsl`(ExampleInt $x) ==> {
    $x++;
  }`;

  ExampleDsl`() ==> {
    $j = 0;
    for ($i = 0; $i < 10; $i++) {
      $j = $j + $i;
    }
    return $j;
  }`;

  ExampleDsl`(MyClassWithPostfixPlusPlus $i) ==> {
    $i++;
  }`;
}
