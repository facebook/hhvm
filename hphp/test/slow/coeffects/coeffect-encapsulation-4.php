<?hh

<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx X as [write_props];

class X {
  const ctx C = [X];
}

function poly<reify T as X>()[T::C]: void {
  echo "poly called\n";
}

function badpure()[]: void {
  poly<X>();
  echo "badpure called\n";
}

<<__EntryPoint>>
function main(): void {
  poly<X>();
  badpure();
  echo "done\n";
}
