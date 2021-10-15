<?hh

<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx X as [write_props];

function xf()[X]: void {
  echo "xf called\n";
}

function inner((function ()[X]: void) $f)[X]: void {
  $f();
  echo "inner called\n";
}
function poly((function ()[_]: void) $f)[ctx $f]: void {
  $f();
  echo "poly called\n";
}

function badpure()[]: void {
  poly(xf<>);
}

<<__EntryPoint>>
function main(): void {
  inner(xf<>);
  poly(xf<>);
  badpure();
  echo "done\n";
}
