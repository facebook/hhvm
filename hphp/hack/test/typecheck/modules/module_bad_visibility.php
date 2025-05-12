//// modules.php
<?hh

new module A {}
//// test.php
<?hh

module A;

class A {
  private internal function bad1(): void {}
  public internal function bad2(): void {}
  private protected internal function bad3(): void {}
  internal function good1(): void {}
  protected internal function good2(): void {}
  internal protected function good3(): void {}

  private internal int $bad1 = 42;
  public internal int $bad2 = 42;
  private protected internal int $bad3 = 42;
  internal int $good1 = 42;
  protected internal int $good2 = 42;
  internal protected int $good3 = 42;
}
