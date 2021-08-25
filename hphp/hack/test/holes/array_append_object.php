<?hh

/* HH_FIXME[2084] */
function array_append_object(classname<_> $yolo): void {
  $obj = new $yolo();
  /* HH_FIXME[4006] */
  $obj[] = true;
}
