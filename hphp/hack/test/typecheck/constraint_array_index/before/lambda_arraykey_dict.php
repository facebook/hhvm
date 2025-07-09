<?hh
function f(): void {
  $f = ($v ==> $v[0]);
  $f(dict["a" => 0, 0 => 0]);
}
