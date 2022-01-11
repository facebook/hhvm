<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C extends Exception { }

<<__SupportDynamicType>>
function test(bool $b, vec<~C> $c, dynamic $d):void {
  $x = $c[0];
  if ($b) throw $x;
  else throw $d;
}
