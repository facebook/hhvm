<?hh
<<file:__EnableUnstableFeatures('typed_local_variables', 'shape_destructure')>>

<<__EntryPoint>>
function main(): void {
  let $x: int = 0;
  shape ('a' => $x, 'b' => $y) = shape('a' => "s", 'b' => 1);
}
