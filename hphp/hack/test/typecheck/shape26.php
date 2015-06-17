<?hh

/**
 * Shape unification
 */

type s = shape('x' => int);

type t = shape(
  'x' => int,
  'y' => ?string,
);

function test(bool $b, s $s, t $t) {
  if ($b) {
    $u = shape('x' => 4);
  } else {
    $u = shape('y' => 'aaa');
  }

  hh_show($u); // disjoint field sets, does not unify

  if ($b) {
    $u = shape('x' => 4);
  } else {
    $u = shape('x' => 'aaa');
  }

  hh_show($u); // common field set, unifies

  if ($b) {
    $u = $s;
  } else {
    $u = $t;
  }

  hh_show($u); // missing optional field in declared shape, does not unify

  if ($b) {
    $u = shape('x' => 4);
  } else {
    $u = $t;
  }

  hh_show($u); // shape with known and unknown fields, does not unify

}
