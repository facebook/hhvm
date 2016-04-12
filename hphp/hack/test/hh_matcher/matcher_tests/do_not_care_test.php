//// tosearch.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function foo(): void { }

  private function baz(): int { return 2; }

  public function bar(): bool { return false; }

  protected function bar(): void { return; }

  public function baz(): void { echo "hi"; }

  private function baz(): void { }
}

//// matcherpattern.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class __SOMENODE {
  public function __KSTAR() : void {}

  public function __SOMENODE() : void {
    return "__ANY";
  }

  public function __KSTAR() : void {}
}
