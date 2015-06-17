<?hh

/**
 * Shape with unknown fields cannot be a subtype of a shape with known fields
 */

interface I<-T> {}

function f<Tv>(Tv $_): I<Tv> {
  // UNSAFE
}

type s = shape(
  'x' => int,
  'y' => string,
);

function test(): I<s> {
  $s = shape('x' => 3);
  $s = f($s);
  return $s;
}
