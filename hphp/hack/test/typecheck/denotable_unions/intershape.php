<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

type S1 = shape('a' => int);
type S2 = shape('b' => string);
type S12 = (S1 & S2);

function foo_wrong(S12 $s): (int,string) {
  return tuple($s['a'], $s['b']);
}

type OS1 = shape('a' => int, ...);
type OS2 = shape('b' => string, ...);
type OS12 = (OS1 & OS2);

function foo_right(OS12 $s): (int,string) {
  return tuple($s['a'], $s['b']);
}

function bar(OS12 $s): OS1 {
  return $s;
}

function good():void {
  $p = foo_right(shape('a' => 1, 'b' => "A"));
}

function bad():void {
  $p = foo_wrong(shape('a' => 1, 'b' => "A"));
}
