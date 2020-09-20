<?hh

<<__EntryPoint>>
function entrypoint_1488(): void {

  if (isset($g)) {
    include '1488-1.inc';
  }
  else {
    include '1488-2.inc';
  }
  include '1488-classes.inc';
  Z::bar();
}
