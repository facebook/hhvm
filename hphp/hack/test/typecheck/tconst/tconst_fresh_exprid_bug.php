<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function mymap<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}

abstract class LSCS {
  abstract const type TLiteral;
  public function primary(): this {
    return $this;
  }
  abstract public function samey(string $name): this;
  public function getit(): this::TLiteral {
    throw new Exception("A");
  }
  public function withit(
    this::TLiteral $_,
  ): this {
    return $this;
  }
  public function hasit(): bool {
    return false;
  }

}

function expectLSC(LSCS $c):void { }

function foo(vec<LSCS> $vc):void {
  mymap(
    $vc,
     ($c) ==> {
      $new_column = $c->samey("a");
  if ($c->hasit()) {
    $d = $c->getit();
    $new_column->withit($d);
  }
  return $new_column; });
}
