<?hh

final class Foo<reify T> {}

class MyTypeClass {
  const type TShape = shape('x' => int);
}

class TupleClass {
  const type TTuple = (int, string);
}

class MultiTypeClass {
  const type TShape = shape('a' => int, 'b' => string);
  const type TTuple = (int, string, bool);
  const type TSimple = int;
}

class NestedClass {
  const type TNestedShape = shape('inner' => shape('x' => int));
  const type TShapeWithTuple = shape('tuple' => (int, string));
}

<<__EntryPoint>>
function main(): void {
  $x = new Foo<MyTypeClass::TShape>();
  echo "Expecting to see true:\n";
  var_dump($x is Foo<shape('x' => int)>);
  echo "Success!\n";

  echo "\nTesting tuple type constant:\n";
  $tuple = new Foo<TupleClass::TTuple>();
  var_dump($tuple is Foo<(int, string)>);

  var_dump($tuple is Foo<(string, int)>);

  echo "\nTesting multiple type constants:\n";
  $shape = new Foo<MultiTypeClass::TShape>();
  $tuple2 = new Foo<MultiTypeClass::TTuple>();
  $simple = new Foo<MultiTypeClass::TSimple>();

  var_dump($shape is Foo<shape('a' => int, 'b' => string)>);
  var_dump($tuple2 is Foo<(int, string, bool)>);
  var_dump($simple is Foo<int>);

  echo "\nTesting type constant shape against non-shape:\n";
  var_dump($simple is Foo<shape('x' => int)>);

  echo "\nTesting type constant tuple against non-tuple:\n";
  var_dump($simple is Foo<(int, string)>);

  echo "\nTesting nested structures:\n";
  $nested = new Foo<NestedClass::TNestedShape>();
  var_dump($nested is Foo<shape('inner' => shape('x' => int))>);

  echo "\nAll type constant tests passed!\n";
}
