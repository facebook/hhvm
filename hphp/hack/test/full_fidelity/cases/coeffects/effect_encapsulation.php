<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newctx X as [a] = [b];

newctx Y as [a, b] = [c];

newctx Z as [a] = [b, c];

newctx W as [] = [a];

newctx U as [a] = [];
