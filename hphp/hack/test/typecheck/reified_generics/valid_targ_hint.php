<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<
  reify Tdog,
  Tcat,
  reify Trex,
> {
  public function f<
    reify Tfox
  >(C<Tfox, Tdog, Tcat> $c): void {}
}
