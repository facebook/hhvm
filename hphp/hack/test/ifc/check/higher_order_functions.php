<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("PUBLIC")>>
  public int $value = 0;
}

function apply((function(int): int) $f, C $c): void {
  // This is fine since we're inferring flows
  $c->value = $f(0);
}

<<Cipp>>
function cipp_apply((function(int): int) $f, C $c): void {
  // This is illegal because we must assume that $f is cipp
  $c->value = $f(0);
}

<<Cipp>>
function cipp_call((function(int): int) $f): int {
  return $f(1);
}
