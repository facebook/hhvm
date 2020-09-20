<?hh // strict

type s = shape('x' => int, 'y' => int, 'z' => int, ?'t' => int);
type sdots = shape('x' => int, 'y' => int, 'z' => int, ?'t' => int, ...);
type t = shape('x' => string, ?'y' => string);
type tdots = shape('x' => string, ?'y' => string, ...);

function test(bool $b, s $s, t $t, sdots $sdots, tdots $tdots): void {
  if ($b) {
    $x = $s;
  } else {
    $x = $t;
  }
  hh_show($x);

  if ($b) {
    $x = $s;
  } else {
    $x = $tdots;
  }
  hh_show($x);

  if ($b) {
    $x = $s;
  } else {
    $x = $tdots;
    Shapes::removeKey(inout $x, 't');
  }
  hh_show($x);

  if ($b) {
    $x = shape('x' => 1, 'y' => 2, 'z' => 3);
  } else {
    $x = $tdots;
  }
  hh_show($x);

  if ($b) {
    $x = shape('x' => 1, 'y' => 2);
  } else {
    $x = $sdots;
  }
  hh_show($x);
}
