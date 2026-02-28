<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test_function(): void {
  ExampleDsl`wrongCtor {}`;
  ExampleDsl`exampleKeyedCollection {}`;
  ExampleDsl`Map {}`;
  ExampleDsl`dict {}`;
  ExampleDsl`Vector {}`;
  ExampleDsl`symbolType {1 => 2}`;
  ExampleDsl`ExampleKeyedCollection {1, 2}`;
}
