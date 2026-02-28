<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class MyPair<T1, T2> {
  public function __construct(private T1 $fst, private T2 $snd) {}
  public function fst(): T1 { return $this->fst; }
  public function snd(): T2 { return $this->snd; }
}

function dup<T>(T $x): MyPair<T, T> {
  return new MyPair($x, $x);
}

function test(shape('x' => int) $s): MyPair<shape(), shape('x' => int)> {
  $p = dup($s);
  $s1 = $p->fst();
  $s2 = $p->snd();
  Shapes::removeKey(inout $s1, 'x');
  return new MyPair($s1, $s2);
}
