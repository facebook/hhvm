<?hh

<<file: __EnableUnstableFeatures('context_alias_declaration_short')>>

newctx MyCtx as [];

<<__Memoize(#MakeICInaccessible)>>
function memo_fn1($a, $b)[MyCtx]: string {
  return "args: $a, $b\n";
}

<<__Memoize(#NotKeyedByICAndLeakIC__DO_NOT_USE)>>
function memo_fn2($a, $b)[MyCtx]: string {
  return "args: $a, $b\n";
}

<<__EntryPoint>>
function main(): mixed{
  echo memo_fn1(1, 2);
  echo memo_fn2(1, 2);
}
