<?hh // strict

final class MyParent {
  public function foo(): void { echo "myparent\n"; }
}

<<__MockClass>>
final class MyChild extends MyParent {
  public function foo(): void { echo "mychild\n"; }
}
