<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expectBool(bool $b):void { }
function expectFloat(float $f):void { }
function expectString(string $s):void { }

class Inv<T> {
  public function __construct(public T $item) { }
}

function test_array_get_on_tvar(
  shape('a' => bool) $s,
  (int,string) $t,
  Vector<int> $vector,
  Vector<string> $vector2,
): void {
  // Turns a concrete type into a lower bound on a Tvar
  $s = (new Inv($s))->item;
  expectBool($s["a"]);

  $t = (new Inv($t))->item;
  expectString($t[1]);

  $vector = (new Inv($vector))->item;
  $vector[0] = 3;

  $vector2 = (new Inv($vector2))->item;
  $vector2[] = "a";
}
