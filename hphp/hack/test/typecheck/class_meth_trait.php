//// file1.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait MyTestTrait {
  final protected static async function genTest(): Awaitable<void> {}
}

//// file2.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class MyTestClass {
  use MyTestTrait;
  public static function derp(): void {
    self::genTest<>;
  }
}
