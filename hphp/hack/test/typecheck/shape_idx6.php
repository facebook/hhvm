<?hh // strict

interface A {}
interface B {}

interface AB extends A, B {}

/**
 * When field *might* be set, allow indexing into it
 */
function test(bool $cond, A $a, B $b, AB $ab): void {
  if ($cond) {
    $s = shape('x' => $a);
  } else {
    $s = shape('y' => $b);
  }

  hh_show(Shapes::idx($s, 'x'));
  hh_show(Shapes::idx($s, 'x', $a));
  hh_show(Shapes::idx($s, 'y'));
  hh_show(Shapes::idx($s, 'y', $b));

  if ($cond) {
    $s = shape('x' => $a);
  } else {
    $s = shape('x' => $b);
  }

  hh_show(Shapes::idx($s, 'x'));
  hh_show(Shapes::idx($s, 'x', $ab));

  // This looks like something we could detect as an error, but doing it would
  // require a lot of special casing, manual expanding of typedefs and
  // traversing of Tunresolved to ban this case but allow the above ones.
  $s = shape('x' => 4);
  Shapes::idx($s, 'y');
}
