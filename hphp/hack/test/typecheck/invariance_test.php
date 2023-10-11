<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function my_array_keys<Tk as arraykey, Tv>(KeyedContainer<Tk, Tv> $input): vec<Tk> {
  return vec[];
}

function my_array_values<Tv>(Container<Tv> $input): vec<Tv> {
  return vec[];
}

function expectVectorMixed(Vector<mixed> $vm): void {}

function expectVecString(vec<string> $vs): void {}
function genProcess(dict<string, bool> $scuba_data): void {
  $mids = my_array_keys($scuba_data);
  // $mids should have type vec<string>
  // Now although Vector is invariant, we have vec<string> <: vec<mixed>
  // So this should be fine
  // Indeed this is ok if we write Vector<mixed>
  // But if we write Vector and implement invariant generics by both-ways
  // subtyping, then we end up inferring Vector<string> here
  expectVectorMixed(new Vector($mids));
  $x = my_array_values($mids);
  expectVecString($mids);
}
