<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectInt(?int $x):void { }
function expectString(?string $s):void { }
<<__EntryPoint>>
function main(): void {
  $f = () ==> {
    $y1 = yield 'foo';
    $y2 = yield 'bar';
    expectInt($y1);
    expectString($y2);
  };
  $g = ($f)();
  $g->next();
  $g->send('a');
  $g->send(3);
}
