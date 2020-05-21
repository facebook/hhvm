<?hh

abstract final class StackOverflow { public static $g; }
function cmp($a, $b) {
  $g = StackOverflow::$g;
  usort(inout $g, fun('cmp'));
  StackOverflow::$g = $g;
  fiz();
}

function fiz() {
  var_dump(1);
}
<<__EntryPoint>>
function entrypoint_stack_overflow(): void {

  StackOverflow::$g = varray[1,2,3];

  cmp(0, 0);
}
