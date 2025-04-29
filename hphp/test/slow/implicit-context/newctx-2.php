<?hh

<<file: __EnableUnstableFeatures('context_alias_declaration_short')>>

newctx MyCtx as [];

<<__Memoize(#KeyedByIC)>>
function memo_fn1($a, $b)[MyCtx]: void {
}
