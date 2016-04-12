//// tosearch.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function foo(): void { }

  public function baz(): void { }

  public function bar(): void { }

  public function bar(): void { }

  public function baz(): void { }

  public function baz(): void { }
}

//// matcherpattern.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function __KSTAR_ANY() : void { }

  public function bar() : void { }

  public function baz() : void { }

  public function __KSTAR_ANY() : void { }
}
