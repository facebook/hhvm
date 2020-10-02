<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo<Tk as arraykey,Tv>(KeyedContainer<Tk,Tv> $x, Tk $k) : Tv {
  return $x[$k];
}

function expectArraykey(arraykey $b):void { }
function expectFloat(float $f):void { }
function expectNum(num $n):void { }
function expectString(string $s):void { }
function expectKeysetOfArraykey(keyset<arraykey> $ks):void { }
function expectSetOfInt(Set<int> $s):void { }

// Let's just check that ground subtyping against KeyedContainer works as expected
function test_KeyedContainer_subtype(
  // Vector-like stuff
  varray<int> $a,
  vec<int> $v,
  Vector<num> $vector,
  keyset<string> $ks,
): void {
  $v[] = "a";
  expectArraykey($v[0]);
  $vector[] = 2.3;
  expectNum($vector[0]);
  $ks[] = 3;
  expectArraykey($ks[0]);
}

class Inv<T> {
  public function __construct(public T $item) { }
}

function test_array_append_on_tvar(
  bool $b,
  // Vector-like stuff
  varray<int> $a,
  vec<int> $v,
  Vector<num> $vector,
  keyset<string> $ks,
  Set<int> $set,
  varray<int> $varray,
): void {
  $varray = (new Inv($varray))->item;
  $varray[] = "a";
  expectArraykey($varray[0]);
  $v = (new Inv($v))->item;
  $v[] = "a";
  expectArraykey($v[0]);
  $vector = (new Inv($vector))->item;
  $vector[] = 2.3;
  expectNum($vector[0]);
  $ks = (new Inv($ks))->item;
  $ks[] = 3;
  expectKeysetOfArraykey($ks);
  $set = (new Inv($set))->item;
  $set[] = 2;
  expectSetOfInt($set);
  // Now test union
  if ($b) {
    $set = $vector;
  }
  $set[] = 5;
}
