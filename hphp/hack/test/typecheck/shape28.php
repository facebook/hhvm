<?hh // partial

/**
 * Shape with unknown fields cannot be a subtype of a shape with known fields
 */

interface I<-T> {}

/* HH_FIXME[4336] */
function f<Tv>(Tv $_): I<Tv> {

}

type s = shape(
  'x' => int,
  'y' => string,
);

function test(): I<s> {
  $s = shape('x' => 3);
  $s = f($s);
  // OK so $s has type I<shape('x' => int)>
  // But this is a subtype of I<shape('x' => int, 'y' => string)>
  // Because shape('x' => int, 'y' => string) <: shape('x' => int)
  // So we're ok!
  return $s;
}
