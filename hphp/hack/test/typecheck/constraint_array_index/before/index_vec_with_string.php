<?hh
function f(): void {
  $f = ($v ==> $v["moo"]);
  $f(vec[0]);
}
