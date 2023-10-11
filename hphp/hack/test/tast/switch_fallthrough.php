<?hh

function test(): void {
  // $x is an int
  $x = 0;
  hh_show($x);
  switch ($x) {
    case 0:
      // This is a noop fallthrough so we should not do an intersect
    case 1:
      // $x should be int, that isn't wrapped in an unresolved
      hh_show($x);
      // $x is a string
      $x = '';
      hh_show($x);
      // FALLTHROUGH
    case 3:
      // $x should be string & int
      hh_show($x);
      $x = true;
      // $x should be string & int & bool
      hh_show($x);
      // FALLTHROUGH
    case 4:
      // $x should be int & string
      hh_show($x);
      break;
    case 4:
      // $x should be int
      hh_show($x);
      // $x should be float
      $x = 1.0;
      hh_show($x);
      break;
  }
  // $x should be int & string & float & bool
  hh_show($x);
}
