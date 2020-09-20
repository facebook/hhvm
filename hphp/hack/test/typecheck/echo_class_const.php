<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const string RESET = "hello";

  private static function echoMessage(): void {
    echo self::RESET;
  }
}
