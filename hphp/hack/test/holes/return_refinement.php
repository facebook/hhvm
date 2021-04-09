<?hh

function return_refinement():bool {
  $x = 3;
  if(true) {
    /* HH_FIXME[4110] */
    return $x;
  } else {
    return $x;
  }
}
