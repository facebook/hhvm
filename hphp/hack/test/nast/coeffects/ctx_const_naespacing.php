<?hh

<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx X as [write_props];

abstract class A {
  const ctx C = [read_globals, X];
  abstract const ctx C2 as [read_globals, X] = [read_globals, X];
}
