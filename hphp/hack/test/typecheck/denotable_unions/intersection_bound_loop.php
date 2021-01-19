<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface K {
  abstract const type TC as K;
}

interface J {
  abstract const type TC as (K & this);
  public function get(): this::TC;
}

function testit(J $x): void {
  $y = $x->get();
  $y->get();
  $y->get();
}
