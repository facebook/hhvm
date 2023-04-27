<?hh


function foreach_nullable(?vec<int> $xs) : void {
  $y = 0;
  /* HH_FIXME[4110] */
  foreach($xs as $x) {
    $y += $x;
  }
}
