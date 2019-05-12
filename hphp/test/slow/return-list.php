<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class Test {
  public static function getCreateListError(): (int, int) {

    // Not legal
    // This should be "return tuple(0, 1)".
    // "list() can only be used as an lvar. Did you mean to use tuple()?"
    return list(0, 1);
  }
}
<<__EntryPoint>> function main(): void {
Test::getCreateListError();
}
