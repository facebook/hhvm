<?hh

abstract final class StackOverflow { public static $g; }

StackOverflow::$g = array(1,2,3);
function cmp($a, $b) {
  $g = StackOverflow::$g;
  usort(&$g, fun('cmp'));
  StackOverflow::$g = $g;
  fiz();
}

cmp(0, 0);

function fiz() {
  var_dump(1);
}
