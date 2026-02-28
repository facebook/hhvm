<?hh
function f(): void {
  $f = ($v ==> $v[0]);
  $f(Vector{Vector{0}->toImmVector()}->toImmVector());
}
