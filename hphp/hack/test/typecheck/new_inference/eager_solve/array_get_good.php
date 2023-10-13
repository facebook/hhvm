<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo<Tk as arraykey,Tv>(KeyedContainer<Tk,Tv> $x, Tk $k) : Tv {
  return $x[$k];
}

function expectBool(bool $b):void { }
function expectFloat(float $f):void { }
function expectString(string $s):void { }

// Let's just check that ground subtyping against KeyedContainer works as expected
function test_KeyedContainer_subtype(
  // Vector-like stuff
  varray<bool> $a1,
  vec<bool> $v,
  Vector<bool> $vector,
  ConstVector<bool> $cvector,
  ImmVector<bool> $ivector,
  // Map-like stuff
  darray<string,float> $a2,
  dict<string,float> $d,
  Map<string,float> $map,
  ImmMap<string,float> $imap,
  ConstMap<string,float> $cmap,
  keyset<string> $ks
): void {
  expectBool(foo($a1, 42));
  expectBool(foo($v, 42));
  expectBool(foo($vector, 42));
  expectBool(foo($cvector, 42));
  expectBool(foo($ivector, 42));
  expectFloat(foo($a2, "a"));
  expectFloat(foo($d, "a"));
  expectFloat(foo($map, "a"));
  expectFloat(foo($imap, "a"));
  expectFloat(foo($cmap, "a"));
  expectString(foo($ks, "a"));
}

class Inv<T> {
  public function __construct(public T $item) { }
}

function test_array_get_on_tvar(
  // Vector-like stuff
  varray<bool> $a1,
  vec<bool> $v,
  Vector<bool> $vector,
  ConstVector<bool> $cvector,
  ImmVector<bool> $ivector,
  // Map-like stuff
  darray<string,float> $a2,
  dict<string,float> $d,
  Map<string,float> $map,
  ImmMap<string,float> $imap,
  ConstMap<string,float> $cmap,
  keyset<string> $ks
): void {
  // Turns a concrete type into a lower bound on a Tvar
  $a1 = (new Inv($a1))->item;
  expectBool($a1[42]);
  $v = (new Inv($v))->item;
  expectBool($v[42]);
  $vector = (new Inv($vector))->item;
  expectBool($vector[42]);
  $cvector = (new Inv($cvector))->item;
  expectBool($cvector[42]);
  $ivector = (new Inv($ivector))->item;
  expectBool($ivector[42]);
  $a2 = (new Inv($a2))->item;
  expectFloat($a2["a"]);
  $d = (new Inv($d))->item;
  expectFloat($d["a"]);
  $map = (new Inv($map))->item;
  expectFloat($map["a"]);
  $imap = (new Inv($imap))->item;
  expectFloat($imap["a"]);
  $cmap = (new Inv($cmap))->item;
  expectFloat($cmap["a"]);
  $ks = (new Inv($ks))->item;
  expectString($ks["a"]);
}
