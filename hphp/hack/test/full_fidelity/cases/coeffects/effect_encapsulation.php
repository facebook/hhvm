<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype X as [a] = [b];

newtype Y as [a, b] = [c];

newtype Z as [a] = [b, c];

newtype W as [] = [a];

newtype U as [a] = [];
