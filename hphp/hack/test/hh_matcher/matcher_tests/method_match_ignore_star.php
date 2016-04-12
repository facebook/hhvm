//// tosearch.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function foo(): void { }
}

//// matcherpattern.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function __KSTAR_ANY() : void { }

  public function foo() : void { }
}
