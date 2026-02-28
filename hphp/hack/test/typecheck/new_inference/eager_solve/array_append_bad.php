<?hh
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
  Map<int,num> $m,
): void {
  // Turns a concrete type into a lower bound on a Tvar
  $a = (new Inv($a))->item;
  $a[] = "a";
  expectString($a[0]);
  $v = (new Inv($v))->item;
  $v[] = "a";
  expectString($v[0]);
  $vector = (new Inv($vector))->item;
  $vector[] = true;
  $set = (new Inv($set))->item;
  $set[] = true;
  $m = (new Inv($m))->item;
  $m[] = Pair{2, "a"};
  $m[] = Pair{"a", 2};
}
