<?hh

function f() :mixed{
 return true;
}
<<__EntryPoint>>
function entrypoint_1880(): void {
  if (f()) {
    include '1880-1.inc';
  } else {
    include '1880-2.inc';
  }

  include '1880-classes.inc';

  $b = new B;
  $b->g();
}
