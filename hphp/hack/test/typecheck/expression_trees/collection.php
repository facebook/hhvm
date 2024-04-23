<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test_function(): void {
  ExampleDsl`ExampleKeyedCollection {}`;
  ExampleDsl`ExampleKeyedCollectionMut {}`;
  ExampleDsl`ExampleKeyedCollection {1 => 2}`;
  ExampleDsl`ExampleKeyedCollectionMut {1 => 2}`;
  $x = ExampleDsl`1`;
  ExampleDsl`ExampleKeyedCollection {'a' => ${$x}, 'b' => 2}`;
  ExampleDsl`ExampleKeyedCollection {1 => 2, 'a' => ${$x}}`;
  ExampleDsl`ExampleKeyedCollection {1 => null, 'a' => ${$x}}`;
  ExampleDsl`ExampleKeyedCollection {1 => null, 'a' => ${$x}, 'b' => ""}`;
  $y = ExampleDsl`ExampleKeyedCollection {1 => null, 'a' => ${$x}, 'b' => ""}`;
}
