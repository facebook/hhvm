<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test():bool { return false; }

type Tshape = shape(
  ?'a' => int,
  ?'b' => int,
  ?'c' => int,
  ?'d' => int,
  ?'e' => int,
  ?'f' => int,
  ?'g' => int,
  ?'h' => int,
  ?'i' => int,
  ?'j' => int,
  ?'k' => int,
  ?'l' => int,
  ?'m' => int,
  ?'n' => int,
  ?'o' => int,
  ?'p' => int,
  ?'q' => int,
  ?'r' => int,
  ?'s' => int,
  ?'t' => int,
  ?'u' => int,
  ?'v' => int,
  ?'w' => int,
  ?'x' => int,
  ?'y' => int,
  ?'z' => int,
  ?'aa' => int,
  ?'ab' => int);
function badboy2():Tshape {
  $shape = shape('a' => 1, 'b' => 2, 'c' => 3, 'd' => 4, 'e' => 5, 'f' => 6,
                 'g' => 7, 'h' => 8, 'i' => 9, 'j' => 10, 'k' => 11, 'l' => 12,
                 'm' => 13, 'n' => 14, 'o' => 15, 'p' => 16, 'q' => 17,
                 'r' => 18, 's' => 19, 't' => 20, 'u' => 21, 'v' => 22,
                 'w' => 23, 'x' => 24, 'y' => 25, 'z' => 26, 'aa' => 27,
                 'ab' => 28);
  if (!test()) {
    Shapes::removeKey(inout $shape, 'a');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'b');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'c');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'd');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'e');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'f');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'g');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'h');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'i');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'j');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'k');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'l');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'm');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'n');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'o');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'p');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'q');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'r');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 's');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 't');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'u');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'v');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'w');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'x');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'y');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'z');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'aa');
  }
  if (!test()) {
    Shapes::removeKey(inout $shape, 'ab');
  }
  hh_show($shape);
  //hh_show_env();
  return $shape;
}
