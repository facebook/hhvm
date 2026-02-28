<?hh
function f(): void {
  $f = ($v ==> $v[0] + $v["a"]);
  $f(vec[0]);
}
