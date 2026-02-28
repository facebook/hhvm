<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

class C<T> {}

<<__EntryPoint>>
function main() : void {
  let $x : int = 1;
  let $y : C<vec<int>> = new C();
}
