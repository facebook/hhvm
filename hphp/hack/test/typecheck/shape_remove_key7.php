<?hh

/**
 * Sets of removed fields must be compatible
 */

interface I<-T> {}

function f<Tv>(Tv $_): I<Tv> {
  // UNSAFE
}

type s = shape(
  'x' => int,
  'y' => string,
);

type t = shape('x' => int);

function test(t $s): I<s> {
  Shapes::removeKey($s, 'z');
  $s = f($s);
  return $s;
}
