<?hh
function f(): void {
  $f = ($v ==> $v["a"]);
  $f(Vector{0}->toImmVector());
}
