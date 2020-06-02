<?hh

<<__EntryPoint>>
function entrypoint_1722(): void {

  if (isset($g)) {
    include '1722-1.inc';
  }
  else {
    include '1722-2.inc';
  }

  include '1722.y.inc';

  $y = new Y(1,2);
}
