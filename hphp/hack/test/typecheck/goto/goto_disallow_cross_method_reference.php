<?hh // partial

class Foo {

  public function labelDeclaration(): void {
    L0:
  }

  public function labelReference(): void {
    goto L0;
  }
}
