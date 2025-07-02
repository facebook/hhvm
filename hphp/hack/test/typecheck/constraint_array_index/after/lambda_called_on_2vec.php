<?hh
function f(): void {
  $f = (($v0, $v1) ==> $v0[0] + $v1[0]);
  $f(vec[0], vec[0]);
}
