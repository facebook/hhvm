<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<+T> {
  public function __construct(T $_) {}
}

function id<T>(C<T> $c): C<T> {
  return $c;
}

function test(): void {
  $c = id(id(new C(42)));
  hh_show($c);
}
