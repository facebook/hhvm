<?hh
function f(): void {
  $f = ($v ==> $v["a"]);
  $f(Map {0 => 0});
}
