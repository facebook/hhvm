<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function firstx<T>(Traversable<T> $traversable): T {
  throw new Exception("a");
}

function first_keyx<Tk, Tv>(KeyedTraversable<Tk, Tv> $traversable): Tk {
  throw new Exception("a");
}

function takes_int(int $x):void { }
function takes_string(string $x):void { }

function testit():void {
  $d = dict['string' => 0];

  krsort(inout $d);
  takes_int(firstx($d));
  takes_string(first_keyx($d));
}
