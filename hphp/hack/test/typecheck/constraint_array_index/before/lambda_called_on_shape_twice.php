<?hh
function f(): void {
  $f = ($v ==> $v['a'] + $v['b']);
  $f(shape('a' => 0, 'b' => 0));
}
