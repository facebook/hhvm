<?hh
// (c) Meta Platforms, Inc. and affiliates.

interface ISource {
  const type T = int;
}

trait TSource {
  require implements ISource;
}

abstract class Grandparent implements ISource {
  use TSource;
}

trait TGrandparent {
  require extends Grandparent;
}

abstract class Parentt extends Grandparent {
  use TGrandparent;
}

trait TParent {
  require extends Parentt;
  const type T = int;
}

final class Child extends Parentt {
  use TParent;
}
