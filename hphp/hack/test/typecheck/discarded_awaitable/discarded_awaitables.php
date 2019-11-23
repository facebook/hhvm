<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class MyClassName {

  public function test(bool $cond): void {
    $awaitable = $cond ? null : async { return true; };
    if ($awaitable) {}
    if ($awaitable === null) {}
    if ($awaitable !== null) {}
    if ($awaitable is null) {}
    if ($awaitable is nonnull) {}
  }

}
