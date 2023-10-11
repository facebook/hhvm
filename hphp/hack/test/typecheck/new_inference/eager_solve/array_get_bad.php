<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expectBool(bool $b):void { }
function expectFloat(float $f):void { }
function expectString(string $s):void { }

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
  expectBool($a1["a"]);
  $v = (new Inv($v))->item;
  expectBool($v["a"]);
  $vector = (new Inv($vector))->item;
  expectBool($vector["a"]);
  $cvector = (new Inv($cvector))->item;
  expectBool($cvector["a"]);
  $ivector = (new Inv($ivector))->item;
  expectBool($ivector["a"]);
  $a2 = (new Inv($a2))->item;
  expectFloat($a2[42]);
  $d = (new Inv($d))->item;
  expectFloat($d[42]);
  $map = (new Inv($map))->item;
  expectFloat($map[42]);
  $imap = (new Inv($imap))->item;
  expectFloat($imap[42]);
  $cmap = (new Inv($cmap))->item;
  expectFloat($cmap[42]);
  $ks = (new Inv($ks))->item;
  expectString($ks[42]);
}
