<?hh
// Formatting an otherwise-correctly formatted file with trailing newlines
// should not devour the last line before the newlines.
class A {
  public function baz(): int {
    return 5;
  }
}


