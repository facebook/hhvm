<?hh
function f(): void {
  $f = ($v ==> $v['a']);
  $f(shape());
}
