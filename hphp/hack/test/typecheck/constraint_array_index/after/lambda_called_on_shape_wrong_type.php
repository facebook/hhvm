<?hh
function f(): void {
  $f = ($v ==> $v['a'] + 0);
  $f(shape('a' => "meow"));
}
