<?hh

type s = shape(
  'x' => int,
  ...
);

/**
 * Field not listed in declared shape - lint warning to access it
 */
function test(s $s): void {
  Shapes::idx($s, 'y');
}
