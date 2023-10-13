<?hh

interface A {}
interface B {}

interface AB extends A, B {}

type s = shape('x' => A);

type t = shape('x' => B);

function test(bool $cond, s $s, t $t, A $a, AB $ab): void {
  if ($cond) {
    $st = $s;
  } else {
    $st = $t;
  }

  hh_show(Shapes::idx($s, 'x'));
  hh_show(Shapes::idx($s, 'x', $a));
  hh_show(Shapes::idx($st, 'x'));
  hh_show(Shapes::idx($st, 'x', $ab));

}
