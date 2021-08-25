<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newctx X as [a] = [b];

newtype Y as [a] = [b]; // error
