<?hh

function test(
  shape(?'o' => string, ?'p' => string) $x,
): void {
  if ($x is shape('o' => string)) {
  } else {
    if ($x is shape('p' => string)) {
      hh_expect<shape('p' => string)>($x);
    }
  }
}
