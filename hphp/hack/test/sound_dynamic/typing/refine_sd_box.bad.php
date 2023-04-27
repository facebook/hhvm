<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class MyBox<+Tv as supportdyn<mixed>> {
  public function __construct(private Tv $item) { }
  public function get():Tv {
    return $this->item;
  }
}

function foo(supportdyn<nonnull> $x): void {
  $x as MyBox<_>;
  $y = $x->get();
  $y->bogus();
}
