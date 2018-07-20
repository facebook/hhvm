<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Cov<+T> {}

function foo<T as string>(Cov<I<T>> $type): void {}
function bar(Cov<D> $type): void {
  foo($type);
}

abstract class D extends I<this::Tc> {
  abstract const type Tc as string;
}

class I<Ti> {}
