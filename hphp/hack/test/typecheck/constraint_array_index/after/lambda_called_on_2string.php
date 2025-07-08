<?hh
function f(): void {
  $f = (($v, $w) ==> $v[0] . $w[0]);
  $f("abc", "def");
}
