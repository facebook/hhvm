<?hh
function f(): void {
  $f = ($v ==> $v[0] + $v["a"]);
  $f(dict["a" => 0, 0 => 0]);
}
