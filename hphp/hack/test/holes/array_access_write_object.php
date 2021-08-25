<?hh

/* HH_FIXME[2084] */
function array_access_write_object(classname<_> $yolo): void {
  $obj = new $yolo();
  /* HH_FIXME[4370] */
  $obj[1] = true;
}
