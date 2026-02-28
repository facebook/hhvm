<?hh
function f(): void {
  $f = ($v ==> $v[0][0]);
  $f(Vector{Vector{0}});
}
