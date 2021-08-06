<?hh

/* HH_FIXME[2084] */
function array_access_read_object(classname<_> $yolo) : void {
  $x = new $yolo();
  /* HH_FIXME[4005] */
  $x[1];
}
