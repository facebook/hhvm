<?hh
function f(): void {
  $f = ($v ==> $v[0][0][0]);
  $f(vec[dict[0 => keyset[0]]]);
}
