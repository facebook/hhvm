<?hh

function return_sync(): Pair<int, bool> {
  $x = Pair {2, 'true'};
  /* HH_FIXME[4110] */
  return $x;
}
