<?hh

<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx X as [write_props];

function wp()[write_props]: void {
  echo "write_props called\n";
}

function xf()[X]: void {
  wp();
  echo "xf called\n";
}

function badpure()[]: void {
  xf();
}

<<__EntryPoint>>
function main(): void {
  xf();
  badpure();
  echo "done\n";
}
