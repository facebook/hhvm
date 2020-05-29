<?hh // partial

/**
 * Sets of removed fields must be compatible
 */

interface I<-T> {}

function f<Tv>(Tv $_): I<Tv> {
  throw new Exception();
}

type s = shape(
  'x' => int,
  'y' => string,
);

type tt = shape('x' => int);

function test(tt $s): I<s> {
  Shapes::removeKey(inout $s, 'z');
  $s = f($s);
  // This is legal. See shape28.php
  return $s;
}
