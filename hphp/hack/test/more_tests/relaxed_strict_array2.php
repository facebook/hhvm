<?hh // strict

class Foo {
  // We cannot allow Tany to be created in a strict file. This disallows
  // untemplated arrays
  public function bar(): array {
    return array();
  }
}
