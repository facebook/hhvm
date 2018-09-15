<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T> {
  public function __construct(private T $item) { }
}

function test(string $s): void {
  $a = new C<int>($s);
}
