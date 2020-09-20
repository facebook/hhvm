<?hh

<<__EntryPoint>>
function entrypoint_1486(): void {

  if (isset($g)) {
    include '1486-1.inc';
  }
  else {
    include '1486-2.inc';
  }
  include '1486-classes.inc';
  $x = new d;
  $x->t2();
}
