//// tosearch.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function foo(): void { }

  public function bar(): void { }
}

//// matcherpattern.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function __KSTAR_ANY() : void { }

  public function foo() : void { }

  public function __KSTAR() : void { }

  public function bar() : void { }

  public function __KSTAR() : void { }
}
