<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function g(): void {
  ExampleDsl`{
    $f = (ExampleShape<shape('x' => ExampleInt, 'y' => ExampleString)> $shape) ==> 3;
    $f(shape('x' => 2, 'y' => 3));

    // class constants are not allowed as shape keys
    shape(MyConstantClass::KEY => 3);
  }`;
}

class MyConstantClass {
  const string KEY = "key";
}
