//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
new module A {}
//// test.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>
module A;


class A {
  private internal function bad1(): void {}
  protected internal function bad2(): void {}
  public internal function bad3(): void {}
  internal function good(): void {}
}
