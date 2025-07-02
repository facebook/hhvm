<?hh
function f(): void {
  $f = ($v ==> $v[$v]);
  $f(vec[0]);
}
