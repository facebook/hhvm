//// modules.php
<?hh

new module A {}
//// test.php
<?hh

module A;


class A {
  private internal function bad1(): void {}
  protected internal function bad2(): void {}
  public internal function bad3(): void {}
  internal function good(): void {}
}
