<?hh

// We should warn when a file has syntax errors and do best-effort outline

class ValidClass {
  public function method(): void {}
}

enum BadEnum: string {
  FOO = "foo";

  public function invalid(): void {
    // invalid syntax, enums cannot have methods
  }
}

class ClassWithPrivateConst {
  // Invalid: private const in class
  private const int INVALID = 1;
  public function method(): void {}
}

function missing_semicolon(): void {
    echo "test" /* missing semicolon */
}
