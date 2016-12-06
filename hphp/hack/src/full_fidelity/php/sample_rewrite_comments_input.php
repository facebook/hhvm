<?hh
// Copyright 2016-present Facebook. All Rights Reserved.

/**
 * Here's a big delimited comment.
 */
abstract class FooBar {
  public function whatever(array<mixed> $stuff, ?array $things) : this {
    // Some single line comments
    // Some single line comments
    // Some single line comments
    $this->doStuff /* small delimited comment */ ($inputs, $things);
  }
} // One final comment
