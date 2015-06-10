<?hh //strict

function test(bool $b): void {
  if ($b) {
    $f = $x ==> 3;
  } else {
    $f = $y ==> 'aa';
  }

  hh_show($f);
}
