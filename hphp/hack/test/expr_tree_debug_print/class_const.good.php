<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  const FOO = "BAR";

  public function test(): void {
    ExampleDsl`MyClass::FOO`;
  }
}
