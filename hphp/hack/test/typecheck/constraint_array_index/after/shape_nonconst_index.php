<?hh
function f(): void {
  $f = ($v ==> $v['a' . 'b']);
  $f(shape('ab' => 0));
}
