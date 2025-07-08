<?hh
function f(): void {
  $f = ($v ==> $v[""]);
  $f("abc");
}
