<?hh
function f(): void {
  $f = ($v ==> $v[0]->x + $v[0]);
  $f(vec[]);
}
