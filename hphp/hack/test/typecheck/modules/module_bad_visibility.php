<?hh

<<file:__EnableUnstableFeatures('modules')>>
module A;

new module A {}

class A {
  private internal function bad1(): void {}
  protected internal function bad2(): void {}
  public internal function bad3(): void {}
  internal function good(): void {}
}
