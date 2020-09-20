<?hh

<<__EntryPoint>>
function entrypoint_1470(): void {

  $a = 1;
  if ($a) {
    include '1470-1.inc';
  }
  else {
    include '1470-2.inc';
  }
  if ($a) {
    include '1470-3.inc';
  }
  else {
    include '1470-4.inc';
  }

  include '1470-classes.inc';
}
