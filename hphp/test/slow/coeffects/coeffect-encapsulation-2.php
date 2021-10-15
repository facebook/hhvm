<?hh

<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx X as [write_props];

class A {
  const ctx C = [X];

  public function m()[this::C]: void {
    echo "m called\n";
  }
}

function poly(A $a)[$a::C]: void {
  echo "poly called\n";
}

function badpure(A $a)[]: void {
  $a->m();
  echo "badpure called\n";
}

<<__EntryPoint>>
function main(): void {
  $a = new A();
  $a->m();
  poly($a);
  badpure($a);
  echo "done\n";
}
