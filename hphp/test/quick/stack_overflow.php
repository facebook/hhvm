<?hh

abstract final class StackOverflow { public static $g; }
function cmp($a, $b) :mixed{
  $g = StackOverflow::$g;
  usort(inout $g, cmp<>);
  StackOverflow::$g = $g;
  fiz();
}

function fiz() :mixed{
  var_dump(1);
}
<<__EntryPoint>>
function entrypoint_stack_overflow(): void {

  StackOverflow::$g = vec[1,2,3];

  cmp(0, 0);
}
