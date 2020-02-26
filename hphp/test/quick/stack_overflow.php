<?hh

abstract final class StackOverflow { public static $g; }

StackOverflow::$g = varray[1,2,3];
function cmp($a, $b) {
  $g = StackOverflow::$g;
  usort(inout $g, fun('cmp'));
  StackOverflow::$g = $g;
  fiz();
}

cmp(0, 0);

function fiz() {
  var_dump(1);
}
